#!/usr/bin/env python
import sys, os, os.path
import gtk
import gtksourceview2 as gtksourceview
import gio
import pango
import gobject
import vte
import serial
from time import sleep
import usb.core
import usb.util
import numpy as np
import openmv

ex_source =\
'''from openmv import sensor, imlib
while (True):
  image = sensor.snapshot()
  r= imlib.detect_color(image, (340, 50, 50), 10)
  imlib.draw_rectangle(image, r)
'''
def rgb2hsv(r, g, b):
    r, g, b = r/255.0, g/255.0, b/255.0
    mx = max(r, g, b)
    mn = min(r, g, b)
    df = mx-mn
    if mx == mn:
        h = 0
    elif mx == r:
        h = (60 * ((g-b)/df) + 360) % 360
    elif mx == g:
        h = (60 * ((b-r)/df) + 120) % 360
    elif mx == b:
        h = (60 * ((r-g)/df) + 240) % 360
    if mx == 0:
        s = 0
    else:
        s = df/mx
    v = mx
    return int(h), int(s*100), int(v*100)

class OMVGtk:
    def __init__(self):
        #Set the Glade file
        self.builder = gtk.Builder()
        self.builder.add_from_file("openmv-ide.glade")

        sourceview = self.builder.get_object('gtksourceview')

        self.buffer = gtksourceview.Buffer()
        mgr = gtksourceview.style_scheme_manager_get_default()
        style_scheme = mgr.get_scheme('classic')
        if style_scheme:
            self.buffer.set_style_scheme(style_scheme)
        lang_manager = gtksourceview.language_manager_get_default()
        self.buffer.set_highlight_syntax(True)
        self.buffer.set_language(lang_manager.get_language("python"))
        self.buffer.set_text(ex_source)
        sourceview.set_buffer(self.buffer)

        self.terminal = self.builder.get_object('terminal')
        self.fd = os.open("/dev/ttyACM0", os.O_RDWR)
        self.terminal.set_size(80,24)
        self.terminal.set_pty(self.fd)

        self.framebuffer = self.builder.get_object("framebuffer_image")
        # status bar stuff
        self.statusbar = self.builder.get_object("statusbar")
        self.statusbar_ctx = self.statusbar.get_context_id("default")

        # init openmv
        openmv.init()

        #connect signals
        signals = {
            "on_execute_clicked" : self.execute_clicked,
            "on_stop_clicked" : self.stop_clicked,
            "on_top_window_destroy" : self.quit,
            "on_motion_notify": self.motion_notify,
            "on_button_press": self.button_pressed
        }
        self.builder.connect_signals(signals)
        self.window = self.builder.get_object("top_window")

    def execute_clicked(self, widget):
        buf = self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter())
        # interrupt any running code
        self.terminal.feed_child("\x03")
        sleep(0.1)
        # exec script
        openmv.exec_script(buf)

    def stop_clicked(self, widget):
        self.terminal.feed_child("\x03\r\n")

    def motion_notify(self, widget, event):
        x = int(event.x)
        y = int(event.y)
        pixbuf = self.framebuffer.get_pixbuf()
        if x < pixbuf.get_width() and y < pixbuf.get_height():
            pixel = pixbuf.get_pixels_array()[y][x]
            hsv = "(%d, %d, %d)" %(rgb2hsv(pixel[0], pixel[1], pixel[2]))
            self.statusbar.pop(self.statusbar_ctx)
            self.statusbar.push(self.statusbar_ctx, hsv)

    def button_pressed(self, widget, event):
        x = int(event.x)
        y = int(event.y)
        pixbuf = self.framebuffer.get_pixbuf()
        if x < pixbuf.get_width() and y < pixbuf.get_height():
            pixel = pixbuf.get_pixels_array()[y][x]
            print rgb2hsv(pixel[0], pixel[1], pixel[2])

    def update_fb(self):
        # read framebuffer
        fb = openmv.dump_fb()

        # convert to RGB888 and blit
        pixbuf = gtk.gdk.pixbuf_new_from_array(fb[2].reshape((fb[1], fb[0], 3)), gtk.gdk.COLORSPACE_RGB, 8)
        self.framebuffer.set_from_pixbuf(pixbuf)
        gobject.idle_add(omvgtk.update_fb);

    def quit(self, widget):
        os.close(self.fd)

        openmv.release()

        sys.exit(0)

if __name__ == "__main__":
    omvgtk = OMVGtk()
    omvgtk.window.show_all()
    gobject.idle_add(omvgtk.update_fb);
    gtk.main()
