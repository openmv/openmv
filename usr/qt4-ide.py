#!/usr/bin/env python

from __future__ import print_function

from framebuffer import FrameBuffer
from editor import *
import pydfu
from terminal import *
from PyQt4.QtGui import *
from serial import *
from time import sleep
import sys
import os
import signal
import openmv


class OpenMVConnector(QObject):
    found = pyqtSignal()
    not_found = pyqtSignal()

    def __init__(self):
        QObject.__init__(self)
        self.timer = QTimer()
        self.timer.timeout.connect(self.auto_detect)

    def start(self):
        self.timer.start(500)

    def stop(self):
        self.timer.stop()

    def auto_detect(self):
        if openmv.find():
            print('OpenMV Cam found')
            self.found.emit()
        else:
            self.not_found.emit()


class OpenMVIDE(QMainWindow):
    def __init__(self):
        super(OpenMVIDE, self).__init__()

        # Connector detects camera connect/disconnect
        self.connector = OpenMVConnector()
        self.connector.start()
        # Catch connect / disconnect notifications
        self.connector.found.connect(self.do_connect)
        self.connector.not_found.connect(self.do_disconnect)

        self.scale = 1

        # default working directory
        self.dir = os.path.dirname(os.path.realpath(__file__)) + '/'

        # default icons dir
        self.icon_dir = self.dir + 'icons/'

        # default examples location
        self.example_dir = self.dir + 'examples/'

        # default scripts location
        self.script_dir = self.dir + 'scripts/'

        # Location for binary firmware
        self.flash_dir = self.dir + 'bin/'
        self.flash_file = ''

        # script filename
        self.filename = ''

        # default auto connect behavior
        self.auto_connect = True

        # default auto flash behavior
        self.auto_flash = True

        # Actions

        self.exit_action = QAction(QIcon(self.icon_dir + 'Exit.png'), 'Exit', self)
        self.exit_action.setStatusTip('Exit application')
        self.exit_action.setShortcut('Ctrl+Q')
        self.exit_action.triggered.connect(self.do_quit)

        self.connect_action = QAction(QIcon(self.icon_dir + 'Connect.png'), 'Connect', self)
        self.connect_action.setStatusTip('Connect to camera')
        self.connect_action.triggered.connect(self.do_connect)

        self.reset_action = QAction(QIcon(self.icon_dir + 'Eject.png'), 'Reset', self)
        self.reset_action.setStatusTip('Reset camera and disconnect')
        self.reset_action.triggered.connect(self.do_disconnect)

        self.flash_action = QAction(QIcon(self.icon_dir + 'Lightning.png'), 'Flash', self)
        self.flash_action.setStatusTip('Flash new firmware')
        self.flash_action.triggered.connect(self.do_flash)

        self.run_action = QAction(QIcon(self.icon_dir + 'Run.png'), 'Run', self)
        self.run_action.setStatusTip('Run script')
        self.run_action.triggered.connect(self.do_run)

        self.stop_action = QAction(QIcon(self.icon_dir + 'Stop.png'), 'Stop', self)
        self.stop_action.setStatusTip('Stop script')
        self.stop_action.triggered.connect(self.do_stop)

        self.new_action = QAction(QIcon(self.icon_dir + 'NewDocument.png'), 'New', self)
        self.new_action.setStatusTip('New script')
        self.new_action.triggered.connect(self.do_new)

        self.open_action = QAction(QIcon(self.icon_dir + 'Open.png'), 'Open...', self)
        self.open_action.setStatusTip('Open script')
        self.open_action.setShortcut('Ctrl+O')
        self.open_action.triggered.connect(self.do_open)

        self.save_action = QAction(QIcon(self.icon_dir + 'Save.png'), 'Save', self)
        self.save_action.setStatusTip('Save script')
        self.save_action.setShortcut('Ctrl+S')
        self.save_action.triggered.connect(self.do_save)

        self.save_as_action = QAction(QIcon(self.icon_dir + 'SaveAs.png'), 'Save As...', self)
        self.save_as_action.setStatusTip('Save script as new file')
        self.save_as_action.triggered.connect(self.do_save_as)

        self.zoom_in_action = QAction(QIcon(self.icon_dir + 'ZoomIn.png'), 'Zoom In', self)
        self.zoom_in_action.setStatusTip('Make frame buffer preview bigger')
        self.zoom_in_action.triggered.connect(self.do_zoom_in)

        self.zoom_out_action = QAction(QIcon(self.icon_dir + 'ZoomOut.png'), 'Zoom Out', self)
        self.zoom_out_action.setStatusTip('Make frame buffer preview smaller')
        self.zoom_out_action.triggered.connect(self.do_zoom_out)

        self.zoom_reset_action = QAction(QIcon(self.icon_dir + 'ZoomReset.png'), 'Zoom Reset', self)
        self.zoom_reset_action.setStatusTip('Reset frame buffer preview size')
        self.zoom_reset_action.triggered.connect(self.do_zoom_reset)

        self.auto_flash_action = QAction('Auto-flash', self)
        self.auto_flash_action.setStatusTip('Automatically detect DFU mode and flash firmware')
        self.auto_flash_action.triggered.connect(self.do_auto_flash)

        self.auto_connect_action = QAction('Auto-connect', self)
        self.auto_connect_action.setStatusTip('Automatically connect to OpenMV when detected')
        self.auto_connect_action.triggered.connect(self.do_auto_connect)

        self.toolbar1 = self.addToolBar('toolbar1')
        self.toolbar1.addAction(self.connect_action)
        self.toolbar1.addAction(self.reset_action)
        self.toolbar1.addAction(self.flash_action)
        self.toolbar1.addAction(self.run_action)
        self.toolbar1.addAction(self.stop_action)

        self.toolbar2 = self.addToolBar('toolbar2')
        self.toolbar2.addAction(self.new_action)
        self.toolbar2.addAction(self.open_action)
        self.toolbar2.addAction(self.save_action)

        self.toolbar3 = self.addToolBar('toolbar3')
        self.toolbar3.addAction(self.zoom_in_action)
        self.toolbar3.addAction(self.zoom_reset_action)
        self.toolbar3.addAction(self.zoom_out_action)

        # Enable/disable icons
        self.connected = False
        self.running = False

        self.statusBar()

        # Frame buffer
        self.image = None

        # Menu Bar
        menu_bar = self.menuBar()
        file_menu = menu_bar.addMenu('&File')
        file_menu.addAction(self.new_action)
        file_menu.addAction(self.open_action)
        file_menu.addAction(self.save_action)
        file_menu.addAction(self.save_as_action)
        file_menu.addAction(self.exit_action)

        self.default_height = 600
        self.default_width = 800

        # Geometry
        self.setGeometry(50, 50, self.default_width, self.default_height)

        # Editor
        self.editor = PyEditor(self)
        self.editor.setMinimumWidth(300)
        self.editor.setMinimumHeight(200)

        # FrameBuffer
        self.framebuffer = FrameBuffer()
        self.framebuffer.show()
        self.framebuffer.error.connect(self.do_disconnect)

        # Vertical box for framebuffer
        pvbox = QVBoxLayout()
        pvbox.addWidget(self.framebuffer)
        pvbox.addStretch(1)

        # Horizontal box for editor + framebuffer
        hbox = QHBoxLayout()
        hbox.addWidget(self.editor)
        hbox.addLayout(pvbox)

        self.terminal = SerialTerminal(self)
        self.terminal.setMinimumHeight(100)
        self.terminal.setMaximumHeight(200)
        self.terminal.show()
        self.serial = Serial()
        self.terminal.error.connect(self.do_disconnect)

        # Vertical box for hbox + terminal
        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        vbox.addWidget(self.terminal)
        w = QWidget()
        w.setLayout(vbox)
        self.setCentralWidget(w)

        # UI Statuses
        self.update_ui()

        self.show()

    def update_ui(self):

        ## TODO: enable/disable save icon when file is changed

        self.run_action.setEnabled(self.connected)
        self.stop_action.setEnabled(self.connected and self.running)
        self.connect_action.setEnabled(not self.connected)
        self.reset_action.setEnabled(self.connected)
        self.flash_action.setEnabled(self.connected)
        self.zoom_in_action.setEnabled(self.connected)
        self.zoom_out_action.setEnabled(self.connected)
        self.zoom_reset_action.setEnabled(self.connected)
        if self.connected:
            con = '[connected]'
        else:
            con = '[disconnected]'
        if self.filename:
            fn = os.path.basename(self.filename)
        else:
            fn = 'untitled.py'
        self.setWindowTitle('OpenMV IDE (' + fn + ') - ' + con)

    def do_auto_connect(self):
        self.auto_connect = not self.auto_connect

    def do_connect(self):
        if not self.connected:
            self.connected = True
            try:
                # init OpenMV
                openmv.init()
                sleep(0.2)
                # interrupt any running code
                openmv.stop_script()
                sleep(0.2)
                ## TODO: device config
                self.serial = Serial('/dev/openmvcam', 115200, timeout=1)
                self.terminal.start(self.serial)
            except Exception as e:
                print('error connecting OpenMV Cam: %s' % e)
                self.connected = False
            else:
                self.update_ui()
                self.statusBar().showMessage('OpenMV Cam connected.')
                self.framebuffer.start_updater()

    def do_disconnect(self):
        if self.connected:
            self.connected = False
            self.update_ui()
            self.framebuffer.stop_updater()

            openmv.stop_script()
            sleep(0.2)
            # release OpenMV
            openmv.release()
            sleep(0.2)
            openmv.reset()
            sleep(0.2)

            self.connector.start()
            self.statusBar().showMessage('Camera disconnected.')

            try:
                if self.serial and self.serial.isOpen():
                    print('Disconnecting terminal')
                    self.serial.close()
                    self.terminal.reset()
            except IOError as e:
                print('error disconnecting OpenMV Serial: %s' % e)


    def do_auto_flash(self):
        self.auto_flash = not self.auto_flash

    def do_flash(self):
        self.update_ui()

        if self.connected:
            dialog = QDialog(self)
            filename = QLineEdit()
            chooser = QFileDialog()
            chooser.setDirectory(self.flash_dir)
            browse = QPushButton('&...')
            browse.pressed.connect(chooser.exec_)
            chooser.fileSelected.connect(filename.setText)

            hlayout1 = QHBoxLayout()
            hlayout1.addWidget(filename, stretch=1)
            hlayout1.addWidget(browse)

            ok_button = QPushButton('&Ok')
            cancel_button = QPushButton('&Cancel')

            hlayout2 = QHBoxLayout()
            hlayout2.addWidget(ok_button)
            hlayout2.addWidget(cancel_button)

            vlayout = QVBoxLayout()
            vlayout.addLayout(hlayout1)
            vlayout.addLayout(hlayout2)

            dialog.setLayout(vlayout)
            ok_button.pressed.connect(dialog.accept)
            cancel_button.pressed.connect(dialog.reject)

            dialog.exec_()
            dialog.hide()

            if dialog.result() == QDialog.Accepted:
                print('Accepted')

                self.flash_file = filename.text()
                with open(self.flash_file, 'r') as f:
                    buf = f.read()

                self.flash_update(buf)

            else:
                print('Rejected')

    def flash_update(self, buf):
        flash_offsets = [0x08000000, 0x08004000, 0x08008000, 0x0800C000,
                         0x08010000, 0x08020000, 0x08040000, 0x08060000,
                         0x08080000, 0x080A0000, 0x080C0000, 0x080E0000]

        # Erasing
        total = len(flash_offsets)

        progress = QProgressDialog(self)
        progress.setRange(0, total)
        progress.setValue(0)
        progress.setWindowTitle('Initialize...')
        print('Initialize...')
        progress.setAutoReset(False)
        progress.show()

        # call dfu-util
        openmv.enter_dfu()
        sleep(1.0)

        # Initialize
        pydfu.init()

        progress.setWindowTitle('Erasing...')
        print('Erasing...')
        progress.setValue(0)

        pg = 0
        for offset in flash_offsets:
            progress.setValue(pg)
            pydfu.page_erase(flash_offsets[pg])
            print(pg)
            pg += 1

        offset = 0
        size = len(buf)
        progress.setRange(0, size)
        progress.setWindowTitle('Downloading...')
        print('Downloading...')

        while offset < size:
            progress.setValue(offset)
            pg_size = min(64, size - offset)
            page = buf[offset:offset + pg_size]
            #print pg_size, offset, offset + pg_size, size - offset, size
            pydfu.write_page(page, offset)
            offset += pg_size

        progress.hide()
        # this isn't working...
        openmv.reset()

    def do_run(self):
        buf = str(self.editor.toPlainText())
        # interrupt any running code
        openmv.stop_script()
        sleep(0.1)
        # exec script
        openmv.exec_script(buf)
        self.running = True
        self.update_ui()

    def do_stop(self):
        openmv.stop_script()
        self.running = False
        self.update_ui()

    def do_new(self):
        # TODO: Check for save-as first
        self.editor.setPlainText('')
        self.filename = ''
        self.update_ui()

    def do_open(self):
        # TODO: Check for save-as first
        if os.path.exists(self.script_dir):
            my_dir = self.script_dir
        else:
            my_dir = self.example_dir

        filename = QFileDialog.getOpenFileName(parent=self,
                                               caption=self.tr('Open Micro Python Script'),
                                               directory=my_dir,
                                               filter=self.tr("Python scripts (*.py)"))
        if filename:
            try:
                infile = open(filename, 'r')
                self.editor.setPlainText(infile.read())
                # store new filename
                self.filename = str(filename)
                self.update_ui()
            except (IOError, OSError) as e:
                QErrorMessage(self).showMessage('Error opening file: ' + e)

    def do_save_as(self):
        self.do_save(True)

    def do_save(self, save_as=False):
        if save_as:
            filename = ''
        else:
            filename = self.filename

        # filename will be empty if we haven't saved the document yet
        if not filename:
            filename = QFileDialog.getSaveFileName(parent=self,
                                                   caption=self.tr('Save Micro Python Script'),
                                                   directory=self.script_dir,
                                                   filter=self.tr("Python scripts (*.py)"))
        if filename:
            try:
                outfile = open(filename, 'w')
                outfile.write(self.editor.toPlainText())
                # store new filename
                self.filename = str(filename)
                self.update_ui()
            except (IOError, OSError) as e:
                QErrorMessage(self).showMessage('Error saving file: ' + e)

    def do_quit(self):
        # TODO: check for file save status
        if self.connected:
            self.do_disconnect()
        self.framebuffer.quit()
        QApplication.quit()

    def do_zoom_in(self):
        self.framebuffer.increase_scale(0.5)

    def do_zoom_out(self):
        self.framebuffer.decrease_scale(0.5)

    def do_zoom_reset(self):
        self.framebuffer.reset_scale()

    def interrupt_handler(self, signum, frame):
        print('CTRL-C caught')


def main():
    app = QApplication(sys.argv)

    ide = OpenMVIDE()
    signal.signal(signal.SIGINT, ide.interrupt_handler)
    sys.exit(app.exec_())

    ## TODO: Take script as argument and run straightaway


if __name__ == '__main__':
    main()
