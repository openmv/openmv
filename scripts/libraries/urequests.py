# The MIT License (MIT)
# 
# Copyright (c) 2013, 2014 micropython-lib contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Source: improved version of https://github.com/micropython/micropython-lib/blob/master/python-ecosys/urequests/urequests.py
import usocket
import ubinascii

class Response:
    def __init__(self, code, reason, headers=None, content=None):
        self.encoding = "utf-8"
        self._content = content
        self._headers = headers
        self.reason = reason
        self.status_code = code

    @property
    def headers(self):
        return str(self._headers, self.encoding)

    @property
    def content(self):
        return str(self._content, self.encoding)

    def json(self):
        import ujson
        return ujson.loads(self._content)

def readline(s):
    l = bytearray()
    while True:
        try:
            l += s.read(1)
            if (l[-1] == b'\n'):
                break
        except:
            break
    return l

def socket_readall(s):
    buf = b''
    while True:
        recv = b''
        try:
            recv = s.recv(1)
        except:
            pass
        if len(recv) == 0:
            break
        buf += recv
    return buf

def request(method, url, data=None, json=None, files=None, headers={}, auth=None, stream=None):
    try:
        proto, dummy, host, path = url.split("/", 3)
    except ValueError:
        proto, dummy, host = url.split("/", 2)
        path = ""
    if proto == "http:":
        port = 80
    elif proto == "https:":
        import ussl
        port = 443
    else:
        raise ValueError("Unsupported protocol: " + proto)

    if ":" in host:
        host, port = host.split(":", 1)
        port = int(port)

    if auth:
        headers['Authorization'] = b'Basic %s'%(ubinascii.b2a_base64('%s:%s' %(auth[0], auth[1]))[0:-1])

    resp_code = 0
    resp_reason = None
    resp_headers = []

    ai = usocket.getaddrinfo(host, port)[0]
    s = usocket.socket(ai[0], ai[1], ai[2])
    try:
        s.connect(ai[-1])
        s.settimeout(5.0)
        if proto == "https:":
            s = ussl.wrap_socket(s, server_hostname=host)

        s.write(b"%s /%s HTTP/1.0\r\n" % (method, path))

        if not "Host" in headers:
            s.write(b"Host: %s\r\n" % host)

        # Iterate over keys to avoid tuple alloc
        for k in headers:
            s.write(k)
            s.write(b": ")
            s.write(headers[k])
            s.write(b"\r\n")

        if json is not None:
            import ujson
            data = ujson.dumps(json)
            s.write(b"Content-Type: application/json\r\n")

        if files is not None:
            data = bytearray()
            boundary = b"37a4bcce91521f74142f1868e328a6b9"
            s.write(b"Content-Type: multipart/form-data; boundary=%s\r\n"%(boundary))
            for name, fileobj in files.items():
                data += b"--%s\r\n"%(boundary)
                data += b'Content-Disposition: form-data; name="%s"; filename="%s"\r\n\r\n' %(name, fileobj[0])
                data += fileobj[1].read()
                data += b"\r\n"
            data += b"\r\n--%s--\r\n"%(boundary)

        if data:
            s.write(b"Content-Length: %d\r\n\r\n" % len(data))
            s.write(data)
        else:
            s.write(b"\r\n")

        response = socket_readall(s).split(b"\r\n")
        while response:
            l = response.pop(0).strip()
            if not l or l == b"\r\n":
                break
            if l.startswith(b"Transfer-Encoding:"):
                if b"chunked" in l:
                    raise ValueError("Unsupported " + l)
            elif l.startswith(b"Location:") and not 200 <= status <= 299:
                raise NotImplementedError("Redirects not yet supported")
            if 'HTTPS' in l or 'HTTP' in l:
                sline = l.split(None, 2)
                resp_code = int(sline[1])
                resp_reason = sline[2].decode().rstrip() if len(sline) > 2 else ""
                continue
            resp_headers.append(l)
        resp_headers = b'\r\n'.join(resp_headers)
        content = b'\r\n'.join(response)
    except OSError:
        s.close()
        raise

    return Response(resp_code, resp_reason, resp_headers, content)

def head(url, **kw):
    return request("HEAD", url, **kw)

def get(url, **kw):
    return request("GET", url, **kw)

def post(url, **kw):
    return request("POST", url, **kw)

def put(url, **kw):
    return request("PUT", url, **kw)

def patch(url, **kw):
    return request("PATCH", url, **kw)

def delete(url, **kw):
    return request("DELETE", url, **kw)

