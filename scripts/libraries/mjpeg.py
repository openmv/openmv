# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This is an extension to the mjpeg C user-module.

from umjpeg import *  # noqa
import errno
import re
import socket


class mjpeg_server:
    def _valid_tcp_socket(self):  # private
        if self._tcp_socket is None:
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                try:
                    if hasattr(socket, "SO_REUSEADDR"):
                        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    s.bind(self._myaddr)
                    s.listen(5)
                    s.setblocking(True)
                    self._tcp_socket, self._client_addr = s.accept()
                except OSError:
                    pass
                s.close()
            except OSError:
                self._tcp_socket = None
        return self._tcp_socket is not None

    def _close_tcp_socket(self):  # private
        if self._tcp_socket is not None:
            self._tcp_socket.close()
            self._tcp_socket = None
        if self._playing:
            self._playing = False
            if self._teardown_cb:
                self._teardown_cb(self._pathname)

    def __init__(self, network_if, port=8080):  # private
        self._network = network_if
        self._myip = self._network.ifconfig()[0]
        self._myaddr = (self._myip, port)
        self._tcp_socket = None
        self._setup_cb = None
        self._teardown_cb = None
        self._pathname = ""
        self._playing = False
        self._boundary = "OpenMVCamMJPEG"
        self._extra_headers = []
        print("IP Address:Port %s:%d\nRunning..." % self._myaddr)

    def register_setup_cb(self, cb):  # public
        self._setup_cb = cb

    def register_teardown_cb(self, cb):  # public
        self._teardown_cb = cb

    def set_extra_headers(self, extra_headers):  # public
        self._extra_headers = extra_headers

    def _send_http_response(self, code, name, extra=""):  # private
        self._tcp_socket.send("HTTP/1.0 %d %s\r\n%s\r\n" % (code, name, extra))

    def _send_http_response_ok(self, extra=""):  # private
        self._send_http_response(200, "OK", extra)

    def _parse_mjpeg_request(self, data):  # private
        s = data.decode().splitlines()
        if s and len(s) >= 1:
            line0 = s[0].split(" ")
            request = line0[0]
            self._pathname = re.sub("(http://[a-zA-Z0-9\-\.]+(:\d+)?(/)?)", "/", line0[1])
            if self._pathname != "/" and self._pathname.endswith("/"):
                self._pathname = self._pathname[:-1]
            if line0[2] == "HTTP/1.0" or line0[2] == "HTTP/1.1":
                if request == "GET":
                    self._playing = True
                    self._send_http_response_ok(
                        "Server: OpenMV Cam\r\n"
                        "Content-Type: multipart/x-mixed-replace;boundary=" + self._boundary + "\r\n"
                        "Connection: close\r\n"
                        "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                        "Pragma: no-cache\r\n"
                        "Expires: 0\r\n"
                        + ("\r\n".join(self._extra_headers) if len(self._extra_headers) else "")
                    )
                    if self._setup_cb:
                        self._setup_cb(self._pathname)
                    return
                else:
                    self._send_http_response(501, "Not Implemented")
                    return
        self._send_http_response(400, "Bad Request")

    def _send_mjpeg(self, image_callback, quality):  # private
        try:
            self._tcp_socket.settimeout(5)
            img = image_callback(self._pathname).to_jpeg(quality=quality)
            mjpeg_header = (
                "\r\n--" + self._boundary + "\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length:" + str(img.size()) + "\r\n\r\n"
            )
            self._tcp_socket.sendall(mjpeg_header)
            self._tcp_socket.sendall(img)
        except OSError:
            self._close_tcp_socket()

    def stream(self, image_callback, quality=90):  # public
        while True:
            if self._valid_tcp_socket():
                try:
                    self._tcp_socket.settimeout(0.01)
                    try:
                        data = self._tcp_socket.recv(1400)
                        if data and len(data):
                            self._parse_mjpeg_request(data)
                        else:
                            raise OSError
                    except OSError as e:
                        if e.errno != errno.EAGAIN and e.errno != errno.ETIMEDOUT:
                            raise e
                    if self._playing:
                        self._send_mjpeg(image_callback, quality)
                except OSError:
                    self._close_tcp_socket()
