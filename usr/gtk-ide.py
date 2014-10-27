#!/usr/bin/env python
from array import array
import struct
from PIL import Image
import pydfu
import openmv
import sys, os, os.path
from time import sleep
import gtk, gtksourceview2 as gtksourceview
import gio
import gobject
import vte
import serial
import usb.core
import usb.util
from os.path import expanduser
try:
    # 3.x name
    import configparser
except ImportError:
    # 2.x name
    configparser = __import__("ConfigParser")

IDE_PATH     = os.path.dirname(os.path.realpath(__file__))
GLADE_PATH   = IDE_PATH+"/openmv-ide.glade"
CONFIG_PATH  = IDE_PATH+"/openmv.config"
EXAMPLE_PATH = IDE_PATH+"/examples"
SCRIPTS_PATH = IDE_PATH+"/scripts"

SCALE =1
RECENT_FILES_LIMIT=5
FLASH_OFFSETS= [0x08000000, 0x08004000, 0x08008000, 0x0800C000,
                0x08010000, 0x08020000, 0x08040000, 0x08060000,
                0x08080000, 0x080A0000, 0x080C0000, 0x080E0000]

DEFAULT_CONFIG='''\
[main]
board = openmv1
serial_port = /dev/openmvcam
recent =
'''

class OMVGtk:
    def __init__(self):
        #Set the Glade file
        self.builder = gtk.Builder()
        self.builder.add_from_file(GLADE_PATH)

        # get top window
        self.window = self.builder.get_object("top_window")

        # status bar stuff
        self.statusbar = self.builder.get_object("statusbar")
        self.statusbar_ctx = self.statusbar.get_context_id("default")

        # set buttons
        self.save_button = self.builder.get_object('save_file_toolbutton')
        self.connect_button = self.builder.get_object('connect_button')

        self.save_button.set_sensitive(False)
        self.connect_button.set_sensitive(True)

        # set control buttons
        self.controls = [
            self.builder.get_object('reset_button'),
            self.builder.get_object('bootloader_button'),
            self.builder.get_object('exec_button'),
            self.builder.get_object('stop_button'),
            self.builder.get_object('zoomin_button'),
            self.builder.get_object('zoomout_button'),
            self.builder.get_object('bestfit_button'),
            self.builder.get_object('refresh_button')]

        self.connected = False
        map(lambda x:x.set_sensitive(False), self.controls)

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
        sourceview.set_buffer(self.buffer)

        #configure the terminal
        self.fd = -1
        self.terminal = self.builder.get_object('terminal')
        self.terminal.set_size(80,24)

        # get drawingarea
        self.pixbuf = None
        self.drawingarea = self.builder.get_object("drawingarea")
        self.da_menu = self.builder.get_object("da_menu")

        # selection coords
        self.sel_ended=False
        self.selection_started=False
        self.x1 = self.y1 = self.x2 = self.y2 =0

        # set control scales attributes
        self.builder.get_object("contrast_adjust").attr=    openmv.ATTR_CONTRAST
        self.builder.get_object("brightness_adjust").attr=  openmv.ATTR_BRIGHTNESS
        self.builder.get_object("saturation_adjust").attr=  openmv.ATTR_SATURATION
        self.builder.get_object("gainceiling_adjust").attr= openmv.ATTR_GAINCEILING

        #connect signals
        signals = {
            "on_top_window_destroy"         : self.quit,
            "on_connect_clicked"            : self.connect_clicked,
            "on_reset_clicked"              : self.reset_clicked,
            "on_fwupdate_clicked"           : self.fwupdate_clicked,
            "on_fwpath_clicked"             : self.fwpath_clicked,
            "on_execute_clicked"            : self.execute_clicked,
            "on_stop_clicked"               : self.stop_clicked,
            "on_motion_notify"              : self.motion_notify,
            "on_button_press"               : self.button_pressed,
            "on_button_release"             : self.button_released,
            "on_open_file"                  : self.open_file,
            "on_new_file"                   : self.new_file,
            "on_save_file"                  : self.save_file,
            "on_save_file_as"               : self.save_file_as,
            "on_save_template_activate"     : self.save_template,
            "on_save_descriptor_activate"   : self.save_descriptor,
            "on_ctrl_scale_value_changed"   : self.on_ctrl_scale_value_changed,
            "on_zoomin_clicked"             : self.zoomin_clicked,
            "on_zoomout_clicked"            : self.zoomout_clicked,
            "on_bestfit_clicked"            : self.bestfit_clicked,
            "on_updatefb_clicked"           : self.updatefb_clicked,
        }
        self.builder.connect_signals(signals)

        # create fresh config if needed
        if not os.path.isfile(CONFIG_PATH):
            try:
                with open(CONFIG_PATH, "w") as f:
                    f.write(DEFAULT_CONFIG)
            except (IOError, OSError) as e:
                print ("Failed to create config file %s"%(e))
                sys.exit(1)

        # load config
        self.config = configparser.ConfigParser()
        try:
            self.config.read(CONFIG_PATH)
        except (IOError, OSError) as e:
            print ("Failed to open config file %s"%(e))
            sys.exit(1)

        # current file path
        self.file_path= None
        self.fw_file_path=""
#        path = self.config.get("main", "last_opened_file")
#        if os.path.isfile(path):
#            self._load_file(path)

        # built-in examples menu
        if os.path.isdir(EXAMPLE_PATH):
            submenu = gtk.Menu()
            menu = self.builder.get_object('example_menu')
            files = sorted(os.listdir(EXAMPLE_PATH))
            for f in files:
                if f.endswith(".py"):
                    label = os.path.basename(f)
                    mitem = gtk.MenuItem(label)
                    mitem.connect("activate", self.open_example, EXAMPLE_PATH)
                    submenu.append(mitem)

            menu.set_submenu(submenu)

        # recent files menu
        self.files = []
        files =self.config.get("main", "recent")
        if files:
            self.files = files.split(',')
            self.update_recent_files()


    def show_message_dialog(self, msg_type, msg):
        message = gtk.MessageDialog(parent=self.window, flags=gtk.DIALOG_DESTROY_WITH_PARENT,
                                    type=msg_type, buttons=gtk.BUTTONS_OK, message_format=msg)
        message.run()
        message.destroy()

    def refresh_gui(self, delay=0.0001, wait=0.0001):
        sleep(delay)
        gtk.main_iteration_do(block=False)
        sleep(wait)

    def connect(self):
        self.terminal = self.builder.get_object('terminal')
        try:
            # open VCP and configure the terminal
            self.fd = os.open(self.config.get("main", "serial_port"), os.O_RDWR)
            self.terminal.reset(True, True)
            self.terminal.set_size(80,24)
            self.terminal.set_pty(self.fd)
        except IOError as e:
            self.show_message_dialog(gtk.MESSAGE_ERROR, "Failed to connect to OpenMV\n%s"%e)
            return

        try:
            # init openmv
            openmv.init()

            # interrupt any running code
            openmv.stop_script()
            sleep(0.1)
        except IOError as e:
            self.show_message_dialog(gtk.MESSAGE_ERROR, "Failed to connect to OpenMV\n%s"%e)
            return

        self.connected = True
        self._update_title()
        self.connect_button.set_sensitive(False)
        map(lambda x:x.set_sensitive(True), self.controls)

    def disconnect(self):
        try:
            # close VCP
            os.close(self.fd)
        except OSError:
            pass

        #reset terminal
        self.terminal.set_pty(-1)
        self.terminal.reset(True, True)

        try:
            # stop running code
            openmv.stop_script()
            # release OpenMV
            openmv.release()
        except IOError:
            pass
        finally:
            self.connected = False
            self._update_title()
            self.connect_button.set_sensitive(True)
            map(lambda x:x.set_sensitive(False), self.controls)

    def connect_clicked(self, widget):
        self.connect()

    def fwpath_clicked(self, widget):
        fw_entry = self.builder.get_object("fw_entry")
        dialog = gtk.FileChooserDialog(title=None,action=gtk.FILE_CHOOSER_ACTION_OPEN,
                buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_current_folder(SCRIPTS_PATH)
        ff = gtk.FileFilter()
        ff.set_name("dfu")
        ff.add_pattern("*.bin") #TODO change to DFU
        dialog.add_filter(ff)

        if dialog.run() == gtk.RESPONSE_OK:
            fw_entry.set_text(dialog.get_filename())

        dialog.destroy()

    # Fake multitasking :P
    def fwupdate_task(self, state):
        if (state["init"]):
            pydfu.init()
            state["init"]=False
            state["erase"]=True
            state["bar"].set_text("Erasing...")
            return True
        elif (state["erase"]):
            page = state["page"]
            total = len(FLASH_OFFSETS)
            pydfu.page_erase(FLASH_OFFSETS[page])
            page +=1
            state["bar"].set_fraction(page/float(total))
            if (page == total):
                state["erase"] = False
                state["write"] = True
                state["bar"].set_text("Uploading...")
            state["page"] = page
            return True
        elif (state["write"]):
            buf = state["buf"]
            xfer_bytes = state["xfer_bytes"]
            xfer_total = state["xfer_total"]

            # Send chunk
            chunk = min (64, xfer_total-xfer_bytes)
            pydfu.write_page(buf[xfer_bytes:xfer_bytes+chunk], xfer_bytes)

            xfer_bytes += chunk
            state["xfer_bytes"] = xfer_bytes
            state["bar"].set_fraction(xfer_bytes/float(xfer_total))

            if (xfer_bytes == xfer_total):
                pydfu.exit_dfu()
                state["dialog"].hide()
                return False

            return True

    def fwupdate_clicked(self, widget):
        if (self.connected):
            dialog = self.builder.get_object("fw_dialog")
            fw_entry = self.builder.get_object("fw_entry")
            fw_progress = self.builder.get_object("fw_progressbar")
            ok_button = self.builder.get_object("fw_ok_button")
            cancel_button = self.builder.get_object("fw_cancel_button")

            ok_button.set_sensitive(True)
            cancel_button.set_sensitive(True)
            dialog.set_transient_for(self.window);

            # default FW bin path
            fw_entry.set_text(self.fw_file_path)
            fw_progress.set_text("")
            fw_progress.set_fraction(0.0)

            if dialog.run() == gtk.RESPONSE_OK:
                ok_button.set_sensitive(False)
                cancel_button.set_sensitive(False)

                fw_path = fw_entry.get_text()
                try:
                    with open(fw_path, 'r') as f:
                        buf= f.read()
                except (IOError, OSError) as e:
                    dialog.hide()
                    self.show_message_dialog(gtk.MESSAGE_ERROR, "Failed to open file %s"%str(e))
                    return

                self.fw_file_path = fw_path

                state={"init":True, "erase":False, "write":False,
                    "page":0, "buf":buf, "bar":fw_progress, "dialog":dialog,
                    "xfer_bytes":0, "xfer_total":len(buf)}

                # call dfu-util
                openmv.enter_dfu()
                sleep(1.0)
                gobject.gobject.idle_add(self.fwupdate_task, state);
            else:
                dialog.hide()

    def reset_clicked(self, widget):
        if (self.connected):
            openmv.reset()

    def execute_clicked(self, widget):
        buf = self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter())
        # interrupt any running code
        openmv.stop_script()
        sleep(0.1)
        # exec script
        openmv.exec_script(buf)

    def stop_clicked(self, widget):
        openmv.stop_script();

    def zoomin_clicked(self, widget):
        global SCALE
        SCALE+=1

    def zoomout_clicked(self, widget):
        global SCALE
        if SCALE>1:
            SCALE-=1

    def bestfit_clicked(self, widget):
        global SCALE
        SCALE=1

    def updatefb_clicked(self, widget):
        openmv.fb_update()

    def button_pressed(self, widget, event):
        self.x1 = int(event.x)
        self.y1 = int(event.y)
        self.x2 = int(event.x)
        self.y2 = int(event.y)
        self.selection_started = True

    def button_released(self, widget, event):
        self.x2 = int(event.x)
        self.y2 = int(event.y)
        self.selection_started = False
        self.da_menu.popup(None, None, None, event.button, event.time, None)
        self.da_menu.show_all()


    def motion_notify(self, widget, event):
        x = int(event.x)
        y = int(event.y)
        self.x2 = int(event.x)
        self.y2 = int(event.y)
        if self.pixbuf and x < self.pixbuf.get_width() and y < self.pixbuf.get_height():
            pixel = self.pixbuf.get_pixels_array()[y][x]
            rgb = "(%d, %d, %d)" %(pixel[0], pixel[1], pixel[2])
            self.statusbar.pop(self.statusbar_ctx)
            self.statusbar.push(self.statusbar_ctx, rgb)

    def _rgb(self, rgb):
        return struct.pack("BBB", ((rgb & 0xF800) >> 11)*255/31, ((rgb & 0x07E0)>>5)*255/63, (rgb & 0x001F)*255/31)

    def update_drawing(self):
        if (not self.connected):
            return True

        try:
            # read drawingarea
            b = openmv.fb_get()
            #fb = openmv.fb_dump()
        except IOError as e:
            self.disconnect()
            self._update_title()
            print("%s"%(e))
            return True
        else:
            if b:
                fmt = b[0]
                w = b[1]
                h = b[2]
                buf = b[3]

                if fmt == openmv.FORMAT_GRAY:
                    s = buf.tostring()
                    buff = ''.join([y for yyy in zip(s, s, s) for y in yyy])
                elif fmt == openmv.FORMAT_RGB565:
                    arr = array('H', buf.tostring())
                    arr.byteswap()
                    buff = ''.join(map(self._rgb, arr))
                elif fmt == openmv.FORMAT_JPEG:
                    # JPEG
                    try:
                        buff = Image.frombuffer("RGB", (w, h), buf, "jpeg", "RGB", "").tostring()
                    except Exception as e:
                        print('JPEG decode error (%s)' % e)
                        buff = None
                    else:
                        print len(buff), w, h, w*h*3
                        if len(buff) != w*h*3:
                            print('JPEG buff len != w*h*bpp')
                            buff = None
                elif buf:
                    print('Unknown format %d' % fmt)
                    buff = None
                else:
                    print('empty image data returned')
                    buff = None

                if buff:
                    # create pixbuf from np array
                    self.pixbuf = gtk.gdk.pixbuf_new_from_data(buff, gtk.gdk.COLORSPACE_RGB, False, 8, w, h, w*3)
                    self.pixbuf = self.pixbuf.scale_simple(w*SCALE, h*SCALE, gtk.gdk.INTERP_BILINEAR)

                    self.drawingarea.realize()
                    cm = self.drawingarea.window.get_colormap()
                    gc = self.drawingarea.window.new_gc(foreground=cm.alloc_color('#FFFFFF', True, False))

                    self.drawingarea.set_size_request(w*SCALE, h*SCALE)
                    self.drawingarea.window.draw_pixbuf(gc, self.pixbuf, 0, 0, 0, 0)
                    if self.selection_started or self.da_menu.flags() & gtk.MAPPED:
                        self.drawingarea.window.draw_rectangle(gc, False, self.x1, self.y1, self.x2-self.x1, self.y2-self.y1)

            return True

    def on_ctrl_scale_value_changed(self, adjust):
        openmv.set_attr(adjust.attr, int(adjust.value))

    def save_config(self):
        # TODO set config items from GUI
        #self.config.set("section", "key", value)
        self.config.set("main", "recent", ','.join(self.files))
        with open(CONFIG_PATH, "w") as file:
           self.config.write(file)

    def _update_title(self):
        if (self.file_path==None):
            title = "Untitled"
        else:
            title = os.path.basename(self.file_path)

        title += " [Connected]" if self.connected else " [Disconnected]"
        self.window.set_title(title)


    def update_recent_files(self):
        if (self.file_path and self.file_path not in self.files ):
            self.files.insert(0, self.file_path)

        if len(self.files)>RECENT_FILES_LIMIT:
            self.files.pop()

        submenu = gtk.Menu()
        menu = self.builder.get_object('recent_menu')
        for f in self.files:
            if f.endswith(".py"):
                mitem =gtk.MenuItem(f)
                mitem.connect("activate", self.open_example, "")
                submenu.append(mitem)

        menu.set_submenu(submenu)
        menu.show_all()

    def _load_file(self, path):
        self.file_path = path
        if path == None: # New file
            self.save_button.set_sensitive(True)
            self.buffer.set_text("")
        else:
            self.save_button.set_sensitive(False)
            with open(path, "r") as file:
                self.buffer.set_text(file.read())
            self.update_recent_files()
        self._update_title()

    def _save_file(self, new_file):
        if new_file:
            dialog = gtk.FileChooserDialog(title=None,action=gtk.FILE_CHOOSER_ACTION_SAVE,
                    buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
            dialog.set_default_response(gtk.RESPONSE_OK)
            dialog.set_current_folder(SCRIPTS_PATH)
            ff = gtk.FileFilter()
            ff.set_name("python")
            ff.add_pattern("*.py")
            dialog.add_filter(ff)

            if dialog.run() == gtk.RESPONSE_OK:
                self.file_path = dialog.get_filename()
                self.save_button.set_sensitive(False)
                self._update_title()
                self.update_recent_files()
                with open(dialog.get_filename(), "w") as file:
                    file.write(self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter()))

            dialog.destroy()
        else:
            self.save_button.set_sensitive(False)
            with open(self.file_path, "w") as file:
                file.write(self.buffer.get_text(self.buffer.get_start_iter(), self.buffer.get_end_iter()))

    def save_template(self, widget):
        self.da_menu.hide()
        x = self.x1
        y = self.y1
        w = self.x2-self.x1
        h = self.y2-self.y1

        entry = self.builder.get_object("template_entry")
        image = self.builder.get_object("template_image")
        image.set_from_pixbuf(self.pixbuf.subpixbuf(x, y, w, h))

        dialog = self.builder.get_object("save_template_dialog")
        dialog.set_transient_for(self.window);
        #dialog.set_default_response(gtk.RESPONSE_OK)

        if dialog.run() == gtk.RESPONSE_OK:
            openmv.save_template(x/SCALE, y/SCALE, w/SCALE, h/SCALE, entry.get_text()) #Use Scale
        dialog.hide()

    def save_descriptor(self, widget):
        self.da_menu.hide()
        x = self.x1
        y = self.y1
        w = self.x2-self.x1
        h = self.y2-self.y1

        entry = self.builder.get_object("desc_entry")
        image = self.builder.get_object("desc_image")
        image.set_from_pixbuf(self.pixbuf.subpixbuf(x, y, w, h))

        dialog = self.builder.get_object("save_descriptor_dialog")
        dialog.set_transient_for(self.window);
        #dialog.set_default_response(gtk.RESPONSE_OK)

        if dialog.run() == gtk.RESPONSE_OK:
            #if not entry.get_text():
            openmv.save_descriptor(x/SCALE, y/SCALE, w/SCALE, h/SCALE, entry.get_text()) #Use Scale
        dialog.hide()

    def new_file(self, widget):
        self._load_file(None)

    def save_file(self, widget):
        self._save_file(self.file_path==None)

    def save_file_as(self, widget):
        self._save_file(True)

    def open_file(self, widget):
        dialog = gtk.FileChooserDialog(title=None,action=gtk.FILE_CHOOSER_ACTION_OPEN,
                buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_current_folder(SCRIPTS_PATH)
        ff = gtk.FileFilter()
        ff.set_name("python")
        ff.add_pattern("*.py")
        dialog.add_filter(ff)

        if dialog.run() == gtk.RESPONSE_OK:
            self._load_file(dialog.get_filename())

        dialog.destroy()

    def open_example(self, widget, basedir):
        self.file_path = os.path.join(basedir, widget.get_label())
        self._load_file(self.file_path)

    def text_changed(self, widget):
        self.save_button.set_sensitive(True)

    def quit(self, widget):
        # disconnect
        self.disconnect()

        self.save_config()

        # exit
        sys.exit(0)

if __name__ == "__main__":
    omvgtk = OMVGtk()
    omvgtk.window.show_all()
    gobject.gobject.idle_add(omvgtk.update_drawing)
    gtk.main()
