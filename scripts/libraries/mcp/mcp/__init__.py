# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Model Context Protocol (MCP) server for OpenMV cameras.
#
# Serves MCP over Streamable HTTP (protocol revision 2026-07-28, plus the
# initialize-based 2025-03-26 .. 2025-11-25 revisions for older clients)
# on any lwIP network interface: WiFi, Ethernet or USB-NCM. Responses are
# plain application/json; SSE and sessions are not used.
#
# The library ships a catalog of built-in tools (camera, ML models, codes,
# vision) and resources. Applications add their own with @mcp.tool and
# @mcp.resource.
#
# Usage::
#
#     import asyncio, csi, mcp
#
#     csi0 = csi.CSI()
#     csi0.reset()
#     csi0.pixformat(csi.RGB565)
#     csi0.framesize(csi.QVGA)
#
#     @mcp.tool("Return the number of frames processed so far")
#     def frame_count():
#         return count
#
#     async def main():
#         server = mcp.Server(csi=csi0)
#         asyncio.create_task(server.serve(port=80))
#         while True:
#             img = csi0.snapshot()
#             await asyncio.sleep_ms(0)
#
#     asyncio.run(main())
#
# Tool handlers run inside the event loop, between the application's awaits,
# so they can safely use the camera and models. MicroPython does not keep
# function annotations or defaults, so tool parameters are declared on the
# decorator: name=("type", "description") for a required parameter, or
# name=("type", "description", default) for an optional one. A dict is
# taken as the JSON Schema of the parameter as-is.

import asyncio
import binascii
import gc
import json
import os
import sys
import time

import image

try:
    import omv
except ImportError:
    omv = None

__version__ = "0.1.0"

_MODERN_VERSIONS = ("2026-07-28",)
_LEGACY_VERSIONS = ("2025-11-25", "2025-06-18", "2025-03-26")
_DEFAULT_LEGACY_VERSION = "2025-03-26"

_META_VERSION = "io.modelcontextprotocol/protocolVersion"
_META_SERVER_INFO = "io.modelcontextprotocol/serverInfo"

# JSON-RPC and MCP error codes.
_PARSE_ERROR = -32700
_INVALID_REQUEST = -32600
_METHOD_NOT_FOUND = -32601
_INVALID_PARAMS = -32602
_INTERNAL_ERROR = -32603
_HEADER_MISMATCH = -32020
_UNSUPPORTED_VERSION = -32022

_HTTP_REASONS = {
    200: "OK",
    202: "Accepted",
    400: "Bad Request",
    401: "Unauthorized",
    403: "Forbidden",
    404: "Not Found",
    405: "Method Not Allowed",
    413: "Payload Too Large",
    415: "Unsupported Media Type",
    431: "Request Header Fields Too Large",
    503: "Service Unavailable",
}

# Methods whose Mcp-Name header mirrors a body field.
_NAME_FIELDS = {
    "tools/call": "name",
    "resources/read": "uri",
    "prompts/get": "name",
}

# Placeholder for large base64 payloads. JSON is serialized with this marker
# and the raw base64 bytes are written in its place, which avoids building a
# second copy of the image as a str.
_BLOB_MARK = "@@OMV_MCP_BLOB_%d@@"

_GROUPS = ("system", "camera", "ml", "codes", "vision")


class Error(Exception):
    """Raise from a tool to report a tool execution error to the model."""


class Image:
    """Image content returned by a tool or resource.

    Wraps an image.Image (compressed to JPEG on output) or raw encoded
    bytes with their MIME type.
    """

    def __init__(self, img, quality=90, max_width=0, mime_type="image/jpeg"):
        self.img = img
        self.quality = quality
        self.max_width = max_width
        self.mime_type = mime_type

    def b64(self):
        data = self.img
        if not isinstance(data, (bytes, bytearray, memoryview)):
            data = _to_jpeg(data, self.quality, self.max_width).bytearray()
        return binascii.b2a_base64(data, newline=False)


class Result:
    """A tool result with structured data and optional images."""

    def __init__(self, data=None, images=(), text=None):
        self.data = data
        self.images = images
        self.text = text


class _Tool:
    def __init__(self, name, fn, description, params, title=None, read_only=True):
        self.name = name
        self.fn = fn
        self.description = description
        self.title = title
        self.read_only = read_only
        self.props = {}
        self.required = []
        for pname, spec in params.items():
            if isinstance(spec, dict):
                prop = dict(spec)
                optional = "default" in prop
            else:
                prop = {"type": spec[0]}
                if len(spec) > 1 and spec[1]:
                    prop["description"] = spec[1]
                optional = len(spec) > 2
                if optional:
                    prop["default"] = spec[2]
            # A None default marks an optional parameter with no default:
            # it is left out of the call and the function's default applies.
            if optional and prop["default"] is None:
                del prop["default"]
            if not optional:
                self.required.append(pname)
            self.props[pname] = prop

    def schema(self):
        d = {
            "name": self.name,
            "description": self.description,
            "inputSchema": {
                "type": "object",
                "properties": self.props,
                "additionalProperties": False,
            },
            "annotations": {"readOnlyHint": self.read_only},
        }
        if self.required:
            d["inputSchema"]["required"] = self.required
        if self.title:
            d["title"] = self.title
        return d

    def bind(self, args):
        if not isinstance(args, dict):
            raise Error("arguments must be an object")
        for pname in args:
            if pname not in self.props:
                raise Error("unknown argument: " + pname)
        kwargs = {}
        for pname, prop in self.props.items():
            if pname in args and args[pname] is not None:
                value = args[pname]
                _check_type(pname, value, prop)
                kwargs[pname] = value
            elif pname in self.required:
                raise Error("missing required argument: " + pname)
            elif "default" in prop:
                kwargs[pname] = prop["default"]
        return kwargs


class _Resource:
    def __init__(self, uri, fn, name, description, mime_type):
        self.uri = uri
        self.fn = fn
        self.name = name
        self.description = description
        self.mime_type = mime_type

    def schema(self):
        d = {"uri": self.uri, "name": self.name, "mimeType": self.mime_type}
        if self.description:
            d["description"] = self.description
        return d


_user_tools = {}
_user_resources = {}


def tool(description, name=None, title=None, read_only=True, **params):
    """Decorator that registers an application tool.

    Example::

        @mcp.tool("Count objects above a score", min_score=("number", "Minimum score", 0.5))
        def count_objects(min_score):
            ...
    """
    def wrap(fn):
        n = name or fn.__name__
        _user_tools[n] = _Tool(n, fn, description, params, title, read_only)
        return fn
    return wrap


def resource(uri, description=None, name=None, mime_type="application/json"):
    """Decorator that registers an application resource.

    The function returns a str, bytes, an Image, or any JSON value.
    """
    def wrap(fn):
        _user_resources[uri] = _Resource(uri, fn, name or fn.__name__, description, mime_type)
        return fn
    return wrap


def _check_type(pname, value, prop):
    t = prop.get("type")
    if t == "integer":
        ok = isinstance(value, int) and not isinstance(value, bool)
    elif t == "number":
        ok = isinstance(value, (int, float)) and not isinstance(value, bool)
    elif t == "string":
        ok = isinstance(value, str)
    elif t == "boolean":
        ok = isinstance(value, bool)
    elif t == "array":
        ok = isinstance(value, list)
    elif t == "object":
        ok = isinstance(value, dict)
    else:
        ok = True
    if not ok:
        raise Error("argument %s must be of type %s" % (pname, t))
    if "enum" in prop and value not in prop["enum"]:
        raise Error("argument %s must be one of %s" % (pname, ", ".join(str(e) for e in prop["enum"])))
    if "minimum" in prop and value < prop["minimum"]:
        raise Error("argument %s must be >= %s" % (pname, prop["minimum"]))
    if "maximum" in prop and value > prop["maximum"]:
        raise Error("argument %s must be <= %s" % (pname, prop["maximum"]))


def _to_jpeg(img, quality, max_width):
    if img.format() == image.JPEG:
        return img
    scale = 1.0
    if max_width and img.width() > max_width:
        scale = max_width / img.width()
    return img.to_jpeg(quality=quality, x_scale=scale, y_scale=scale, copy=True)


def _decode_header_value(value):
    # Mcp-Name values that are not plain ASCII are sent as =?base64?...?=
    if value.startswith("=?base64?") and value.endswith("?="):
        return binascii.a2b_base64(value[9:-2]).decode()
    return value


def _rect(obj):
    return [obj.x, obj.y, obj.w, obj.h]


def _round(v, n=2):
    return round(float(v), n)


def _tolist(arr):
    if hasattr(arr, "tolist"):
        return arr.tolist()
    return list(arr)


def _roi_prop():
    return {"type": "array", "items": {"type": "integer"}, "minItems": 4, "maxItems": 4, "default": None,
            "description": "Region of interest [x, y, w, h] in pixels. Default is the whole frame."}


def _annotate_prop():
    return ("boolean", "Also return the frame with the results drawn on it.", False)


class Server:
    """MCP server.

    Args:
        csi: The application's csi.CSI object. Used for snapshots and
            camera settings. Optional if `frame` is given.
        frame: Optional callable returning the image the image tools work
            on (e.g. the app's last frame). Defaults to csi.snapshot().
        name, version, instructions: Reported to clients.
        groups: Built-in tool groups to register. Any of "system", "camera",
            "ml", "codes", "vision". Default is all of them.
        read_only: If True, tools that change camera settings are omitted.
        token: If set, clients must send "Authorization: Bearer <token>".
        origins: Allowed values of the HTTP Origin header. Requests that carry
            any other Origin are rejected (DNS rebinding protection).
        path: The MCP endpoint path.
        max_clients: Maximum concurrent connections. Extra connections wait for
            a free slot, and idle keep-alive connections are closed to make room.
            lwIP has 5 TCP PCBs by default, so keep this below that.
        max_body: Maximum request body size in bytes.
        timeout_ms: Idle timeout for a connection.
        model_dirs: Directories searched for .tflite models.
    """

    def __init__(self, csi=None, frame=None, name="openmv", version=__version__,
                 instructions=None, groups=_GROUPS, read_only=False, token=None,
                 origins=(), path="/mcp", max_clients=4, max_body=16384,
                 timeout_ms=30000, model_dirs=("/rom", "/flash", "/sdcard")):
        self.csi = csi
        self.frame = frame
        self.name = name
        self.version = version
        self.instructions = instructions or self._default_instructions()
        self.read_only = read_only
        self.token = token
        self.origins = origins
        self.path = path
        self.max_clients = max_clients
        self.max_body = max_body
        self.timeout_ms = timeout_ms
        self.model_dirs = model_dirs
        self.clients = 0
        # Tasks blocked waiting for the next request on a keep-alive connection.
        self.idle = set()
        self.model = None
        self.model_key = None
        self.postprocess = None
        self.tools = {}
        self.resources = {}
        for g in groups:
            getattr(self, "_register_" + g)()
        self._register_resources()

    async def serve(self, host="0.0.0.0", port=80):
        """Serve forever. Run with asyncio.create_task(server.serve())."""
        srv = await asyncio.start_server(self._handle, host, port)
        await srv.wait_closed()

    # Registry ###############################################################

    def _add(self, name, fn, description, read_only=True, **params):
        if read_only or not self.read_only:
            self.tools[name] = _Tool(name, fn, description, params, None, read_only)

    def _all_tools(self):
        # Application tools override built-ins with the same name.
        tools = dict(self.tools)
        tools.update(_user_tools)
        return tools

    def _all_resources(self):
        resources = dict(self.resources)
        resources.update(_user_resources)
        return resources

    def _default_instructions(self):
        return ("This is an OpenMV machine vision camera. Image tools work on the current camera "
                "frame. Coordinates are in pixels with the origin at the top-left; rects are "
                "[x, y, w, h]. Use capture_frame to see what the camera sees, and annotate=true "
                "on detection tools to see the results drawn on the frame. The camera runs one "
                "tool at a time; parallel calls are queued and run in order, so prefer "
                "sequential calls.")

    # HTTP ###################################################################

    async def _handle(self, reader, writer):
        # Clients open one connection per parallel request and keep them alive,
        # so make room by closing idle connections, then wait for a free slot.
        # Requests are served one at a time anyway.
        start = time.ticks_ms()
        while self.clients >= self.max_clients:
            if self.idle:
                self.idle.pop().cancel()
            elif time.ticks_diff(time.ticks_ms(), start) >= self.timeout_ms:
                await self._send(writer, 503, b"", close=True)
                await self._close(writer)
                return
            await asyncio.sleep_ms(10)
        self.clients += 1
        # Bytes received but not consumed yet (shared by pipelined requests),
        # and whether a request has been served on this connection.
        buf = [b"", False]
        try:
            while True:
                try:
                    keep_alive = await asyncio.wait_for_ms(self._request(reader, writer, buf),
                                                           self.timeout_ms)
                except asyncio.TimeoutError:
                    break
                if not keep_alive:
                    break
                buf[1] = True
        except (OSError, EOFError):
            pass
        finally:
            self.clients -= 1
            await self._close(writer)

    async def _close(self, writer):
        try:
            writer.close()
            await writer.wait_closed()
        except OSError:
            pass

    async def _recv(self, reader, buf):
        # Each await waits for one scheduler pass (one frame of the app loop),
        # so try the non-blocking socket first and read in large chunks.
        data = reader.s.read(2048)
        if data is None:
            data = await reader.read(2048)
        if not data:
            raise EOFError
        buf[0] += data

    async def _request(self, reader, writer, buf):
        while True:
            end = buf[0].find(b"\r\n\r\n")
            if end >= 0:
                break
            if len(buf[0]) > 4096:
                await self._send(writer, 431, b"", close=True)
                return False
            # Between requests a keep-alive connection is idle and can be closed
            # to make room for a new one. A new connection whose first request
            # has not arrived yet is not idle.
            task = asyncio.current_task() if buf[1] and not buf[0] else None
            if task:
                self.idle.add(task)
            try:
                await self._recv(reader, buf)
            except EOFError:
                return False
            finally:
                if task:
                    self.idle.discard(task)
        lines = buf[0][:end].decode().split("\r\n")
        buf[0] = buf[0][end + 4:]

        try:
            method, target, proto = lines[0].strip().split(" ", 2)
        except ValueError:
            await self._send(writer, 400, b"", close=True)
            return False

        if len(lines) > 33:
            await self._send(writer, 400, b"", close=True)
            return False
        headers = {}
        for h in lines[1:]:
            i = h.find(":")
            if i > 0:
                headers[h[:i].strip().lower()] = h[i + 1:].strip()

        keep_alive = proto == "HTTP/1.1" and headers.get("connection", "").lower() != "close"
        length = int(headers.get("content-length", "0") or "0")

        # Drain the body first so the connection stays usable on errors.
        if length > self.max_body:
            await self._send(writer, 413, b"", close=True)
            return False
        while len(buf[0]) < length:
            await self._recv(reader, buf)
        body = buf[0][:length]
        buf[0] = buf[0][length:]

        if target.split("?")[0] != self.path:
            await self._send(writer, 404, b"", close=not keep_alive)
            return keep_alive

        if method != "POST":
            await self._send(writer, 405, b"", close=not keep_alive, extra="Allow: POST\r\n")
            return keep_alive

        origin = headers.get("origin")
        if origin is not None and origin not in self.origins:
            await self._send_error(writer, 403, None, _INVALID_REQUEST, "Origin not allowed", keep_alive)
            return keep_alive

        if self.token and headers.get("authorization") != "Bearer " + self.token:
            await self._send(writer, 401, b"", close=not keep_alive,
                             extra="WWW-Authenticate: Bearer\r\n")
            return keep_alive

        try:
            msg = json.loads(body)
        except ValueError:
            await self._send_error(writer, 400, None, _PARSE_ERROR, "Parse error", keep_alive)
            return keep_alive

        status, response = self._dispatch(msg, headers)
        gc.collect()

        if response is None:
            await self._send(writer, status, b"", close=not keep_alive)
        else:
            await self._send_json(writer, status, response, keep_alive)
        return keep_alive

    async def _send(self, writer, status, body, close=False, extra="", ctype=None):
        head = "HTTP/1.1 %d %s\r\n" % (status, _HTTP_REASONS.get(status, ""))
        if ctype:
            head += "Content-Type: %s\r\n" % ctype
        head += "Content-Length: %d\r\n" % len(body)
        if close:
            head += "Connection: close\r\n"
        head += extra + "\r\n"
        writer.write(head.encode())
        if body:
            writer.write(body)
        await self._drain(writer)

    async def _drain(self, writer):
        # Stream.write sends right away if the socket can take it, and keeps
        # the rest in out_buf. drain() always yields a scheduler pass, so only
        # call it when something is left to send.
        if writer.out_buf:
            await writer.drain()

    async def _send_json(self, writer, status, response, keep_alive):
        blobs = []
        text = json.dumps(_extract_blobs(response, blobs), separators=(",", ":"))
        response = None
        if not blobs:
            await self._send(writer, status, text.encode(), not keep_alive, ctype="application/json")
            return

        # Write the JSON text around each base64 blob without joining them.
        parts = []
        start = 0
        for i in range(len(blobs)):
            mark = _BLOB_MARK % i
            end = text.find(mark, start)
            parts.append(text[start:end].encode())
            parts.append(blobs[i])
            start = end + len(mark)
        parts.append(text[start:].encode())
        text = None
        length = 0
        for p in parts:
            length += len(p)

        head = "HTTP/1.1 %d %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n" % (
            status, _HTTP_REASONS.get(status, ""), length)
        if not keep_alive:
            head += "Connection: close\r\n"
        writer.write((head + "\r\n").encode())
        for p in parts:
            # If the socket is backed up, flush before the next part so a large
            # base64 blob is not copied into out_buf.
            await self._drain(writer)
            writer.write(p)
        await self._drain(writer)

    async def _send_error(self, writer, status, id, code, message, keep_alive, data=None):
        await self._send_json(writer, status, _error(id, code, message, data), keep_alive)

    # JSON-RPC ###############################################################

    def _dispatch(self, msg, headers):
        if not isinstance(msg, dict) or msg.get("jsonrpc") != "2.0" or not isinstance(msg.get("method"), str):
            return 400, _error(None, _INVALID_REQUEST, "Invalid request")

        method = msg["method"]
        params = msg.get("params") or {}
        if not isinstance(params, dict):
            return 400, _error(msg.get("id"), _INVALID_PARAMS, "params must be an object")

        # Notifications (no id) are accepted and need no reply.
        if "id" not in msg:
            return 202, None

        id = msg["id"]
        meta = params.get("_meta") or {}
        version = meta.get(_META_VERSION) if isinstance(meta, dict) else None
        header_version = headers.get("mcp-protocol-version")

        if version is not None:
            # Modern (2026-07-28 and later): stateless, metadata per request.
            if version not in _MODERN_VERSIONS:
                return 400, _error(id, _UNSUPPORTED_VERSION, "Unsupported protocol version",
                                   {"supported": _MODERN_VERSIONS + _LEGACY_VERSIONS, "requested": version})
            mismatch = self._check_headers(method, params, headers, version)
            if mismatch:
                return 400, _error(id, _HEADER_MISMATCH, mismatch)
            modern = True
        elif method == "initialize":
            modern = False
        else:
            # Legacy (initialize-based) revisions.
            if header_version is None:
                header_version = _DEFAULT_LEGACY_VERSION
            if header_version in _MODERN_VERSIONS:
                return 400, _error(id, _HEADER_MISMATCH, "Header mismatch: request _meta has no protocol version")
            if header_version not in _LEGACY_VERSIONS:
                return 400, _error(id, _UNSUPPORTED_VERSION, "Unsupported protocol version",
                                   {"supported": _MODERN_VERSIONS + _LEGACY_VERSIONS,
                                    "requested": header_version})
            modern = False

        handler = _METHODS.get(method)
        if handler is None:
            return (404 if modern else 200), _error(id, _METHOD_NOT_FOUND, "Method not found: " + method)

        try:
            result = handler(self, params)
        except _RpcError as e:
            return 200, _error(id, e.code, e.message)
        except Exception as e:
            return 200, _error(id, _INTERNAL_ERROR, _exc_text(e))

        if modern:
            result["resultType"] = "complete"
        return 200, {"jsonrpc": "2.0", "id": id, "result": result}

    def _check_headers(self, method, params, headers, version):
        if headers.get("mcp-protocol-version") != version:
            return "Header mismatch: MCP-Protocol-Version does not match _meta"
        if headers.get("mcp-method") != method:
            return "Header mismatch: Mcp-Method does not match method"
        field = _NAME_FIELDS.get(method)
        if field is not None:
            value = headers.get("mcp-name")
            if value is None:
                return "Header mismatch: Mcp-Name is missing"
            try:
                value = _decode_header_value(value)
            except Exception:
                return "Header mismatch: Mcp-Name is not valid base64"
            if value != params.get(field):
                return "Header mismatch: Mcp-Name does not match " + field
        return None

    def _capabilities(self):
        return {"tools": {}, "resources": {}}

    def _server_info(self):
        return {"name": self.name, "version": self.version}

    def _m_initialize(self, params):
        requested = params.get("protocolVersion")
        version = requested if requested in _LEGACY_VERSIONS else _LEGACY_VERSIONS[0]
        return {
            "protocolVersion": version,
            "capabilities": self._capabilities(),
            "serverInfo": self._server_info(),
            "instructions": self.instructions,
        }

    def _m_discover(self, params):
        return {
            "supportedVersions": list(_MODERN_VERSIONS + _LEGACY_VERSIONS),
            "capabilities": self._capabilities(),
            "_meta": {_META_SERVER_INFO: self._server_info()},
            "instructions": self.instructions,
        }

    def _m_ping(self, params):
        return {}

    def _m_tools_list(self, params):
        return {"tools": [t.schema() for t in self._all_tools().values()]}

    def _m_tools_call(self, params):
        name = params.get("name")
        t = self._all_tools().get(name)
        if t is None:
            raise _RpcError(_INVALID_PARAMS, "Unknown tool: %s" % name)
        try:
            value = t.fn(**t.bind(params.get("arguments") or {}))
        except Exception as e:
            return {"content": [{"type": "text", "text": _exc_text(e)}], "isError": True}
        return _tool_result(value)

    def _m_resources_list(self, params):
        return {"resources": [r.schema() for r in self._all_resources().values()]}

    def _m_resources_templates_list(self, params):
        return {"resourceTemplates": []}

    def _m_resources_read(self, params):
        uri = params.get("uri")
        r = self._all_resources().get(uri)
        if r is None:
            raise _RpcError(-32002, "Resource not found: %s" % uri)
        value = r.fn()
        content = {"uri": uri, "mimeType": r.mime_type}
        if isinstance(value, Image):
            content["mimeType"] = value.mime_type
            content["blob"] = value
        elif isinstance(value, (bytes, bytearray)):
            content["blob"] = Image(value, mime_type=r.mime_type)
        elif isinstance(value, str):
            content["text"] = value
        else:
            content["text"] = json.dumps(value)
        return {"contents": [content]}

    # Frame helpers ##########################################################

    def _frame(self):
        if self.frame is not None:
            img = self.frame()
            if img is None:
                raise Error("no frame captured yet")
            return img
        if self.csi is None:
            raise Error("no camera: pass csi= or frame= to mcp.Server()")
        return self.csi.snapshot()

    def _image_result(self, img, items, annotate):
        data = {"count": len(items), "results": items}
        if not annotate:
            return data
        img = img.copy()
        for item in items:
            if "line" in item:
                img.draw_line(item["line"], color=(255, 0, 0), thickness=2)
            elif "circle" in item:
                img.draw_circle(item["circle"], color=(255, 0, 0), thickness=2)
            elif "rect" in item:
                img.draw_rectangle(item["rect"], color=(255, 0, 0), thickness=2)
        return Result(data, [Image(img)])

    # Built-in tools: system #################################################

    def _register_system(self):
        self._add("get_info", self.get_info,
                  "Get the camera board, firmware version, image sensor and memory usage.")
        self._add("list_files", self.list_files,
                  "List files and directories on the camera filesystem.",
                  path=("string", "Directory to list.", "/"))

    def get_info(self):
        info = {"implementation": "%s %s" % (sys.implementation.name, ".".join(str(v) for v in sys.implementation.version[:3]))}
        if omv is not None:
            info["board"] = omv.board_type()
            info["board_id"] = omv.board_id()
            info["firmware"] = omv.version_string()
            info["arch"] = omv.arch()
        info["heap_free"] = gc.mem_free()
        info["heap_used"] = gc.mem_alloc()
        info["uptime_ms"] = time.ticks_ms()
        if self.csi is not None:
            # Not every sensor has a csi constant (PS5520 on the N6), fall back to hex.
            sensor = _const_name("csi", self.csi.cid(), _SENSOR_NAMES)
            info["sensor"] = sensor if isinstance(sensor, str) else "0x%04X" % sensor
            info["width"] = self.csi.width()
            info["height"] = self.csi.height()
        return info

    def list_files(self, path):
        entries = []
        for e in os.ilistdir(path):
            item = {"name": e[0], "type": "dir" if e[1] & 0x4000 else "file"}
            if len(e) > 3 and not (e[1] & 0x4000):
                item["size"] = e[3]
            entries.append(item)
        return {"path": path, "entries": entries}

    # Built-in tools: camera #################################################

    def _register_camera(self):
        self._add("capture_frame", self.capture_frame,
                  "Capture the current camera frame as a JPEG image.",
                  quality={"type": "integer", "minimum": 1, "maximum": 100, "default": 90,
                           "description": "JPEG quality."},
                  max_width=("integer", "Downscale the image to at most this width. 0 keeps the size.", 640))
        if self.csi is None:
            return
        self._add("get_settings", self.get_settings,
                  "Get the camera settings: pixel format, frame size, exposure, gain and orientation.")
        self._add("set_settings", self.set_settings,
                  "Change camera settings. Only the given settings are changed. Returns the new settings.",
                  read_only=False,
                  pixformat={"type": "string", "enum": _available("csi", _PIXFORMAT_NAMES), "default": None},
                  framesize={"type": "string", "enum": _available("csi", _FRAMESIZE_NAMES), "default": None},
                  auto_exposure=("boolean", "Enable automatic exposure.", None),
                  exposure_us=("integer", "Exposure time in microseconds. Disables automatic exposure.", None),
                  auto_gain=("boolean", "Enable automatic gain.", None),
                  gain_db=("number", "Sensor gain in dB. Disables automatic gain.", None),
                  auto_whitebal=("boolean", "Enable automatic white balance.", None),
                  hmirror=("boolean", "Mirror the image horizontally.", None),
                  vflip=("boolean", "Flip the image vertically.", None),
                  framerate=("integer", "Frame rate limit.", None))

    def capture_frame(self, quality, max_width):
        img = self._frame()
        return Result({"width": img.width(), "height": img.height()}, [Image(img, quality, max_width)])

    def get_settings(self):
        c = self.csi
        s = {}
        for key, fn in (("pixformat", lambda: _const_name("csi", c.pixformat(), _PIXFORMAT_NAMES)),
                        ("framesize", lambda: _const_name("csi", c.framesize(), _FRAMESIZE_NAMES)),
                        ("width", c.width), ("height", c.height),
                        ("framerate", c.framerate),
                        ("exposure_us", c.exposure_us),
                        ("gain_db", lambda: _round(c.gain_db())),
                        ("hmirror", c.hmirror), ("vflip", c.vflip)):
            try:
                s[key] = fn()
            except Exception:
                pass
        return s

    def set_settings(self, pixformat=None, framesize=None, auto_exposure=None, exposure_us=None,
                     auto_gain=None, gain_db=None, auto_whitebal=None, hmirror=None, vflip=None,
                     framerate=None):
        import csi
        c = self.csi
        if pixformat is not None:
            c.pixformat(getattr(csi, pixformat))
        if framesize is not None:
            c.framesize(getattr(csi, framesize))
        if exposure_us is not None:
            c.auto_exposure(False, exposure_us=exposure_us)
        elif auto_exposure is not None:
            c.auto_exposure(auto_exposure)
        if gain_db is not None:
            c.auto_gain(False, gain_db=gain_db)
        elif auto_gain is not None:
            c.auto_gain(auto_gain)
        if auto_whitebal is not None:
            c.auto_whitebal(auto_whitebal)
        if hmirror is not None:
            c.hmirror(hmirror)
        if vflip is not None:
            c.vflip(vflip)
        if framerate is not None:
            c.framerate(framerate)
        return self.get_settings()

    # Built-in tools: ml #####################################################

    def _register_ml(self):
        try:
            import ml  # noqa: F401
        except ImportError:
            return
        self._add("list_models", self.list_models,
                  "List the machine learning models (.tflite) stored on the camera, with labels.")
        self._add("run_model", self.run_model,
                  "Run a model on the current frame. Use postprocess to decode detector outputs into "
                  "boxes; with 'none' the raw output is returned as the top_k classification scores.",
                  model=("string", "Model path, e.g. /rom/yolov8n_192.tflite (see list_models)."),
                  postprocess={"type": "string", "enum": ["none"] + [p[0] for p in _POSTPROCESSORS],
                               "default": "none",
                               "description": "Output decoder matching the model's architecture."},
                  threshold={"type": "number", "minimum": 0, "maximum": 1, "default": 0.5,
                             "description": "Minimum detection score."},
                  top_k=("integer", "Number of classification scores to return.", 5),
                  annotate=_annotate_prop())

    def list_models(self):
        models = []
        for d in self.model_dirs:
            try:
                names = os.listdir(d)
            except OSError:
                continue
            for n in names:
                if n.endswith(".tflite"):
                    path = d + "/" + n
                    item = {"path": path}
                    labels = _read_labels(path)
                    if labels is not None:
                        item["labels"] = labels if len(labels) <= 100 else labels[:100] + ["..."]
                    models.append(item)
        return {"models": models}

    def run_model(self, model, postprocess, threshold, top_k, annotate):
        import ml

        key = (model, postprocess)
        if key != self.model_key:
            # Keep one model loaded at a time; models can be large.
            self.model = None
            self.postprocess = None
            gc.collect()
            pp = None
            if postprocess != "none":
                pp = _load_postprocessor(postprocess)(threshold=threshold)
            self.model = ml.Model(model, postprocess=pp)
            self.model_key = key
            self.postprocess = pp
        if self.postprocess is not None:
            self.postprocess.threshold = threshold

        img = self._frame()
        out = self.model.predict([img])
        labels = self.model.labels

        if self.postprocess is None:
            scores = out[0].flatten().tolist()
            ranked = sorted(range(len(scores)), key=lambda i: scores[i], reverse=True)[:top_k]
            return {"scores": [{"label": labels[i] if labels and i < len(labels) else i,
                                "score": _round(scores[i], 3)} for i in ranked]}

        return self._image_result(img, _detections(out, labels), annotate)

    # Built-in tools: codes ##################################################

    def _register_codes(self):
        if hasattr(image.Image, "find_qrcodes"):
            self._add("find_qrcodes", self.find_qrcodes,
                      "Find and decode QR codes in the current frame.",
                      roi=_roi_prop(), annotate=_annotate_prop())
        if hasattr(image.Image, "find_barcodes"):
            self._add("find_barcodes", self.find_barcodes,
                      "Find and decode 1D barcodes (EAN, UPC, Code 128, ...) in the current frame.",
                      roi=_roi_prop(), annotate=_annotate_prop())
        if hasattr(image.Image, "find_datamatrices"):
            self._add("find_datamatrices", self.find_datamatrices,
                      "Find and decode Data Matrix codes in the current frame.",
                      roi=_roi_prop(), annotate=_annotate_prop())
        if hasattr(image.Image, "find_apriltags"):
            self._add("find_apriltags", self.find_apriltags,
                      "Find AprilTags in the current frame, with id, center, rotation and 3D pose.",
                      families={"type": "array",
                                "items": {"type": "string", "enum": _available("image", _TAG_FAMILIES)},
                                "default": ["TAG36H11"], "description": "Tag families to search for."},
                      roi=_roi_prop(), annotate=_annotate_prop())

    def find_qrcodes(self, roi=None, annotate=False):
        img = self._frame()
        items = [{"payload": c.payload, "rect": _rect(c), "version": c.version}
                 for c in img.find_qrcodes(roi=roi)]
        return self._image_result(img, items, annotate)

    def find_barcodes(self, roi=None, annotate=False):
        img = self._frame()
        items = [{"payload": c.payload, "type": _const_name("image", c.type, _BARCODE_NAMES),
                  "rect": _rect(c), "rotation": _round(c.rotation)}
                 for c in img.find_barcodes(roi=roi)]
        return self._image_result(img, items, annotate)

    def find_datamatrices(self, roi=None, annotate=False):
        img = self._frame()
        items = [{"payload": c.payload, "rect": _rect(c), "rotation": _round(c.rotation)}
                 for c in img.find_datamatrices(roi=roi)]
        return self._image_result(img, items, annotate)

    def find_apriltags(self, families=("TAG36H11",), roi=None, annotate=False):
        mask = 0
        for f in families:
            mask |= getattr(image, f)
        img = self._frame()
        items = [{"id": t.id, "family": t.name, "rect": _rect(t), "center": [t.cx, t.cy],
                  "rotation": _round(t.rotation),
                  "translation": [_round(t.x_translation), _round(t.y_translation), _round(t.z_translation)],
                  "decision_margin": _round(t.decision_margin)}
                 for t in img.find_apriltags(roi=roi, families=mask)]
        return self._image_result(img, items, annotate)

    # Built-in tools: vision #################################################

    def _register_vision(self):
        self._add("find_blobs", self.find_blobs,
                  "Find color blobs. Each threshold is [lo, hi] for grayscale or "
                  "[l_lo, l_hi, a_lo, a_hi, b_lo, b_hi] in LAB for color images.",
                  thresholds={"type": "array", "items": {"type": "array", "items": {"type": "integer"}},
                              "description": "List of color thresholds."},
                  invert=("boolean", "Invert the thresholds.", False),
                  pixels_threshold=("integer", "Minimum blob pixel count.", 10),
                  area_threshold=("integer", "Minimum blob bounding box area.", 10),
                  merge=("boolean", "Merge overlapping blobs.", False),
                  roi=_roi_prop(), annotate=_annotate_prop())
        self._add("find_lines", self.find_lines,
                  "Find infinite lines using the Hough transform.",
                  threshold=("integer", "Minimum line magnitude.", 1000),
                  roi=_roi_prop(), annotate=_annotate_prop())
        self._add("find_line_segments", self.find_line_segments,
                  "Find line segments.",
                  merge_distance=("integer", "Merge segments closer than this many pixels.", 0),
                  max_theta_diff=("integer", "Maximum angle difference in degrees for merging.", 15),
                  roi=_roi_prop(), annotate=_annotate_prop())
        self._add("find_circles", self.find_circles,
                  "Find circles using the Hough transform.",
                  threshold=("integer", "Minimum circle magnitude.", 2000),
                  r_min=("integer", "Minimum radius.", 2),
                  r_max=("integer", "Maximum radius. Default is unlimited.", None),
                  roi=_roi_prop(), annotate=_annotate_prop())
        self._add("find_rects", self.find_rects,
                  "Find rectangles (quads).",
                  threshold=("integer", "Minimum rectangle magnitude.", 10000),
                  roi=_roi_prop(), annotate=_annotate_prop())
        self._add("get_statistics", self.get_statistics,
                  "Get image statistics (mean, median, mode, stdev, min, max, quartiles) "
                  "per channel; LAB channels for color images.",
                  roi=_roi_prop())

    def find_blobs(self, thresholds, invert=False, pixels_threshold=10, area_threshold=10,
                   merge=False, roi=None, annotate=False):
        img = self._frame()
        items = [{"rect": _rect(b), "center": [b.cx, b.cy], "pixels": b.pixels,
                  "rotation": _round(b.rotation), "code": b.code, "density": _round(b.density)}
                 for b in img.find_blobs([tuple(t) for t in thresholds], invert=invert, roi=roi,
                                         pixels_threshold=pixels_threshold,
                                         area_threshold=area_threshold, merge=merge)]
        return self._image_result(img, items, annotate)

    def find_lines(self, threshold=1000, roi=None, annotate=False):
        img = self._frame()
        items = [{"line": [l.x1, l.y1, l.x2, l.y2], "magnitude": l.magnitude, "theta": l.theta, "rho": l.rho}
                 for l in img.find_lines(roi=roi, threshold=threshold)]
        return self._image_result(img, items, annotate)

    def find_line_segments(self, merge_distance=0, max_theta_diff=15, roi=None, annotate=False):
        img = self._frame()
        items = [{"line": [l.x1, l.y1, l.x2, l.y2], "length": l.length, "theta": l.theta}
                 for l in img.find_line_segments(roi=roi, merge_distance=merge_distance,
                                                 max_theta_diff=max_theta_diff)]
        return self._image_result(img, items, annotate)

    def find_circles(self, threshold=2000, r_min=2, r_max=None, roi=None, annotate=False):
        img = self._frame()
        items = [{"circle": [c.x, c.y, c.r], "magnitude": c.magnitude}
                 for c in img.find_circles(roi=roi, threshold=threshold, r_min=r_min, r_max=r_max)]
        return self._image_result(img, items, annotate)

    def find_rects(self, threshold=10000, roi=None, annotate=False):
        img = self._frame()
        items = [{"rect": _rect(r), "corners": [list(p) for p in r.corners], "magnitude": r.magnitude}
                 for r in img.find_rects(roi=roi, threshold=threshold)]
        return self._image_result(img, items, annotate)

    def get_statistics(self, roi=None):
        s = self._frame().get_statistics(roi=roi)
        out = {}
        for prefix in ("", "l_", "a_", "b_"):
            for f in ("mean", "median", "mode", "stdev", "min", "max", "lq", "uq"):
                out[prefix + f] = getattr(s, prefix + f)
        return out

    # Built-in resources #####################################################

    def _register_resources(self):
        self.resources["openmv://info"] = _Resource(
            "openmv://info", self.get_info, "info", "Board, firmware, sensor and memory.", "application/json")
        if "list_models" in self.tools:
            self.resources["openmv://models"] = _Resource(
                "openmv://models", self.list_models, "models", "Models stored on the camera.",
                "application/json")
        if self.csi is not None or self.frame is not None:
            self.resources["openmv://frame/latest"] = _Resource(
                "openmv://frame/latest", lambda: Image(self._frame(), 90, 640), "frame",
                "The current camera frame.", "image/jpeg")


class _RpcError(Exception):
    def __init__(self, code, message):
        self.code = code
        self.message = message


_METHODS = {
    "initialize": Server._m_initialize,
    "server/discover": Server._m_discover,
    "ping": Server._m_ping,
    "tools/list": Server._m_tools_list,
    "tools/call": Server._m_tools_call,
    "resources/list": Server._m_resources_list,
    "resources/templates/list": Server._m_resources_templates_list,
    "resources/read": Server._m_resources_read,
}

_PIXFORMAT_NAMES = ("GRAYSCALE", "RGB565", "BAYER", "YUV422", "JPEG", "BINARY")

_FRAMESIZE_NAMES = (
    "QQQVGA", "QQVGA", "QVGA", "VGA", "HQVGA", "HVGA", "WVGA", "WVGA2", "SVGA", "XGA", "WXGA",
    "SXGA", "SXGAM", "UXGA", "HD", "FHD", "QHD", "QXGA", "WQXGA", "WQXGA2",
    "QCIF", "CIF", "QSIF", "SIF",
)

_SENSOR_NAMES = (
    "OV2640", "OV5640", "OV7670", "OV7690", "OV7725", "OV9650", "MT9V022", "MT9V024", "MT9V032",
    "MT9V034", "MT9M114", "BOSON320", "BOSON640", "LEPTON", "HM01B0", "HM0360", "GC2145",
    "GENX320ES", "GENX320", "PAG7920", "PAG7936", "PAJ6100", "FROGEYE2020", "SOFTCSI",
)

_TAG_FAMILIES = (
    "TAG16H5", "TAG25H9", "TAG36H10", "TAG36H11", "TAGCIRCLE21H7", "TAGCIRCLE49H12",
    "TAGCUSTOM48H12", "TAGSTANDARD41H12", "TAGSTANDARD52H13",
)

_BARCODE_NAMES = (
    "EAN2", "EAN5", "EAN8", "UPCE", "ISBN10", "UPCA", "EAN13", "ISBN13", "I25", "DATABAR",
    "DATABAR_EXP", "CODABAR", "CODE39", "PDF417", "CODE93", "CODE128",
)

# (name, module) of the frozen ml postprocessors.
_POSTPROCESSORS = (
    ("BlazeFace", "mediapipe"),
    ("BlazePalm", "mediapipe"),
    ("HandLandmarks", "mediapipe"),
    ("FaceLandmarks", "mediapipe"),
    ("MoveNet", "mediapipe"),
    ("YoloV5", "ultralytics"),
    ("YoloV8", "ultralytics"),
    ("YoloV2", "darknet"),
    ("YoloLC", "darknet"),
    ("Fomo", "edgeimpulse"),
    ("YoloPro", "edgeimpulse"),
)


def _available(module, names):
    m = __import__(module)
    return [n for n in names if hasattr(m, n)]


def _const_name(module, value, names):
    m = __import__(module)
    for n in names:
        if getattr(m, n, None) == value:
            return n
    return value


def _load_postprocessor(name):
    for n, mod in _POSTPROCESSORS:
        if n == name:
            m = __import__("ml.postprocessing." + mod, None, None, (n,))
            return getattr(m, n)
    raise Error("unknown postprocessor: " + name)


def _read_labels(path):
    try:
        with open(path[:path.rfind(".")] + ".txt") as f:
            return [line.rstrip("\n") for line in f]
    except OSError:
        return None


def _detections(out, labels):
    # Postprocessors return either a list of (rect, score[, keypoints]) for
    # single class models, or a list of such lists, one per class.
    items = []
    if not out:
        return items
    groups = [out] if isinstance(out[0], tuple) else out
    for ci, group in enumerate(groups):
        for det in group:
            item = {"label": labels[ci] if labels and ci < len(labels) else ci,
                    "score": _round(det[1], 3), "rect": [int(v) for v in det[0]]}
            if len(det) > 2 and det[2] is not None:
                item["keypoints"] = [[_round(v, 1) for v in kp] for kp in _tolist(det[2])]
            items.append(item)
    return items


def _tool_result(value):
    if isinstance(value, Result):
        data, images, text = value.data, value.images, value.text
    elif isinstance(value, Image):
        data, images, text = None, [value], None
    elif isinstance(value, image.Image):
        data, images, text = None, [Image(value)], None
    elif isinstance(value, str):
        data, images, text = None, (), value
    elif value is None:
        data, images, text = None, (), "OK"
    else:
        data, images, text = value, (), None

    content = []
    result = {"content": content, "isError": False}
    if data is not None:
        if not isinstance(data, dict):
            data = {"result": data}
        result["structuredContent"] = data
        content.append({"type": "text", "text": json.dumps(data)})
    if text is not None:
        content.append({"type": "text", "text": text})
    for img in images:
        content.append({"type": "image", "data": img, "mimeType": img.mime_type})
    return result


def _extract_blobs(obj, blobs):
    # Replace Image objects with placeholders, collecting their base64 bytes.
    if isinstance(obj, Image):
        blobs.append(obj.b64())
        return _BLOB_MARK % (len(blobs) - 1)
    if isinstance(obj, dict):
        return {k: _extract_blobs(v, blobs) for k, v in obj.items()}
    if isinstance(obj, (list, tuple)):
        return [_extract_blobs(v, blobs) for v in obj]
    return obj


def _error(id, code, message, data=None):
    err = {"code": code, "message": message}
    if data is not None:
        err["data"] = data
    return {"jsonrpc": "2.0", "id": id, "error": err}


def _exc_text(e):
    return "%s: %s" % (type(e).__name__, e)
