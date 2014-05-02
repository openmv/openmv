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
from os.path import expanduser
config_path = expanduser("~")+"/.openmvide.config"

class OMVGtk:
    def __init__(self):
        #Set the Glade file
        self.builder = gtk.Builder()
        self.builder.add_from_file("openmv-ide.glade")

        # get top window
        self.window = self.builder.get_object("top_window")

        # status bar stuff
        self.statusbar = self.builder.get_object("statusbar")
        self.statusbar_ctx = self.statusbar.get_context_id("default")

        #save toolbutton
        self.save_button = self.builder.get_object('save_file_toolbutton')
        self.save_button.set_sensitive(False)

        #configure gtksourceview
        sourceview = self.builder.get_object('gtksourceview')
        self.buffer = gtksourceview.Buffer()
        mgr = gtksourceview.style_scheme_manager_get_default()
        style_scheme = mgr.get_scheme('classic')
        if style_scheme:
            self.buffer.set_style_scheme(style_scheme)
        lang_manager = gtksourceview.language_manager_get_default()
        self.buffer.set_highlight_syntax(True)
        self.buffer.set_language(lang_manager.get_language("python"))
        self.buffer.connect("changed", self.text_changed)

        # open last opened file
        if os.path.isfile(config_path):
            with open(config_path, "r") as file:
                self.file_path = file.read()
            if os.path.isfile(self.file_path):
                with open(self.file_path, "r") as file:
                    self.buffer.set_text(file.read())
                    self.window.set_title(os.path.basename(self.file_path))
            else:
                self.file_path = None

        sourceview.set_buffer(self.buffer)

        # open VCP and configure the terminal
        self.terminal = self.builder.get_object('terminal')
        self.fd = os.open("/dev/ttyACM0", os.O_RDWR)
        self.terminal.set_size(80,24)
        self.terminal.set_pty(self.fd)

        self.framebuffer = self.builder.get_object("framebuffer_image")

        #connect signals
        signals = {
            "on_top_window_destroy" : self.quit,
            "on_execute_clicked"    : self.execute_clicked,
            "on_stop_clicked"       : self.stop_clicked,
            "on_motion_notify"      : self.motion_notify,
            "on_button_press"       : self.button_pressed,
            "on_open_file"          : self.open_file,
            "on_save_file"          : self.save_file,
            "on_save_file_as"       : self.save_file_as,
        }
        self.builder.connect_signals(signals)

        # init openmv
        openmv.init()


    def execute_clicked(self, widget):
        buf = self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter())
        # interrupt any running code
        openmv.stop_script()
        sleep(0.1)
        # exec script
        openmv.exec_script(buf)

    def stop_clicked(self, widget):
        openmv.stop_script();

    def motion_notify(self, widget, event):
        x = int(event.x)
        y = int(event.y)
        pixbuf = self.framebuffer.get_pixbuf()
        if x < pixbuf.get_width() and y < pixbuf.get_height():
            pixel = pixbuf.get_pixels_array()[y][x]
            rgb = "(%d, %d, %d)" %(pixel[0], pixel[1], pixel[2])
            self.statusbar.pop(self.statusbar_ctx)
            self.statusbar.push(self.statusbar_ctx, rgb)

    def button_pressed(self, widget, event):
        x = int(event.x)
        y = int(event.y)
        pixbuf = self.framebuffer.get_pixbuf()
        if x < pixbuf.get_width() and y < pixbuf.get_height():
            pixel = pixbuf.get_pixels_array()[y][x]
            print (pixel[0], pixel[1], pixel[2])

    def update_fb(self):
        # read framebuffer
        fb = openmv.dump_fb()

        # convert to RGB888 and blit
        pixbuf = gtk.gdk.pixbuf_new_from_array(fb[2].reshape((fb[1], fb[0], 3)), gtk.gdk.COLORSPACE_RGB, 8)
        pixbuf = pixbuf.scale_simple(fb[0]*2, fb[1]*2, gtk.gdk.INTERP_BILINEAR)
        self.framebuffer.set_from_pixbuf(pixbuf)
        gobject.idle_add(omvgtk.update_fb);

    def open_file(self, widget):
        dialog = gtk.FileChooserDialog(title=None,action=gtk.FILE_CHOOSER_ACTION_OPEN,
                buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_current_folder("./examples/")
        filter = gtk.FileFilter()
        filter.set_name("python")
        filter.add_pattern("*.py")
        dialog.add_filter(filter)

        if dialog.run() == gtk.RESPONSE_OK:
            with open(dialog.get_filename(), "r") as file:
                self.buffer.set_text(file.read())
                self.file_path=dialog.get_filename()
                self.save_button.set_sensitive(False)
                self.window.set_title(os.path.basename(self.file_path))

        dialog.destroy()

    def save_file(self, widget):
        with open(self.file_path, "w") as file:
            file.write(self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter()))

    def save_file_as(self, widget):
        dialog = gtk.FileChooserDialog(title=None,action=gtk.FILE_CHOOSER_ACTION_SAVE,
                buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_current_folder("./examples/")
        filter = gtk.FileFilter()
        filter.set_name("python")
        filter.add_pattern("*.py")
        dialog.add_filter(filter)

        if dialog.run() == gtk.RESPONSE_OK:
            with open(dialog.get_filename(), "w") as file:
                file.write(self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter()))

        dialog.destroy()

    def text_changed(self, widget):
        self.save_button.set_sensitive(True)

    def quit(self, widget):
        # write last opened file
        with open(config_path, "w") as file:
            file.write(self.file_path)

        # stop any running code
        openmv.stop_script();
        # close VCP
        os.close(self.fd)
        # release OpenMV
        openmv.release()
        # exit
        sys.exit(0)

if __name__ == "__main__":
    omvgtk = OMVGtk()
    omvgtk.window.show_all()
    gobject.idle_add(omvgtk.update_fb);
    gtk.main()
