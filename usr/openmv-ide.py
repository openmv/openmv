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

interface = 2;
altsetting= 1;

def image2pixbuf(buff):
    arr = np.fromstring(buff, dtype=np.uint16).newbyteorder('S')
    r = (((arr & 0xF800) >>11)*255.0/31.0).astype(np.uint8)
    g = (((arr & 0x07E0) >>5) *255.0/63.0).astype(np.uint8)
    b = (((arr & 0x001F) >>0) *255.0/31.0).astype(np.uint8)
    arr = np.column_stack((r, g, b))
    arr = arr.reshape((120, 160, 3))
    return gtk.gdk.pixbuf_new_from_array(arr, gtk.gdk.COLORSPACE_RGB, 8) 

ex_source = '''from openmv import sensor
while (True):
  image = sensor.snapshot()'''

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

        # find USB device 
        self.dev = usb.core.find(idVendor=0x0483, idProduct=0x5740)
        if self.dev is None:
            raise ValueError('Device not found')

        # detach kernel driver    
        if self.dev.is_kernel_driver_active(interface):    
            self.dev.detach_kernel_driver(interface)

        # set FB debug alt setting
        self.dev.set_interface_altsetting(interface, altsetting)

        #connect signals
        signals = { 
            "on_execute_clicked" : self.execute_clicked,
            "on_stop_clicked" : self.stop_clicked,
            "on_top_window_destroy" : self.quit,
        }        
        self.builder.connect_signals(signals)
        self.window = self.builder.get_object("top_window")

    def execute_clicked(self, widget):
        buf = self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter())
        # interrupt running code 
        self.terminal.feed_child("\x03")
        for l in buf.splitlines():
            self.dev.ctrl_transfer(0x41, 10, 0, 2, l+'\n', 5000)
        self.dev.ctrl_transfer(0x41, 11, 0, 2, None, 5000)

    def stop_clicked(self, widget):
        self.terminal.feed_child("\x03\r\n")

    def update_fb(self):
        img_size = 160*120*2
        # request snapshot
        self.dev.ctrl_transfer(0xC1, 8, 0, 2, None, 2000)

        # read framebuffer
        buf = self.dev.read(0x83, img_size, interface, 5000)
    
        if len(buf) <img_size:
            print(len(buf))
            return

        # convert to RGB888 and blit
        pixbuf = image2pixbuf(buf)
        self.framebuffer.set_from_pixbuf(pixbuf)
        gobject.idle_add(omvgtk.update_fb);


    def quit(self, widget):
        os.close(self.fd)

        #reset device
        self.dev.reset();
        
        #reattach kernel driver
        self.dev.attach_kernel_driver(interface)
        sys.exit(0)

if __name__ == "__main__":
    omvgtk = OMVGtk()
    omvgtk.window.show_all()
    gobject.idle_add(omvgtk.update_fb);
    gtk.main()
