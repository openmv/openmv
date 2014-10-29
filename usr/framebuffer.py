import string
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import numpy
import openmv
from usb.core import USBError


class FrameBuffer(QLabel):

    error = pyqtSignal(int)

    error_detected = False

    def __init__(self):
        QLabel.__init__(self)

        self.thread = QThread()

        # image scaling
        self.scale = 1.0
        # the object to poll the camera for framebuffer
        self.updater = ImageUpdater()
        # update image whenever the updater tells us to
        self.updater.update.connect(self.do_update)
        # handle errors sent from the updater
        self.updater.error.connect(self.handle_error)
        # move updater into its own thread
        self.updater.moveToThread(self.thread)
        # start the thread
        self.thread.start(QThread.HighPriority)

        # set up a timer to do periodic polling
        self.timer = QTimer(self)
        # connect timeout to updater do_process
        self.timer.timeout.connect(self.updater.do_process)

        self.do_update(QImage(chr(0) * (321 * 241 * 3), 320, 240, QImage.Format_RGB888))

    def quit(self):
        try:
            self.timer.stop()
            self.thread.quit()
            self.thread.wait(500)
        except QErrorMessage as e:
            print('FrameBuffer error quitting %s' % e.message)

    def start_updater(self):
        #print 'start_updater %s' % QThread.currentThreadId()
        error_detected = False
        self.timer.start(17)

    def stop_updater(self):
        #print 'stop_updater %s' % QThread.currentThreadId()
        self.timer.stop()

    def handle_error(self, error):
        assert isinstance(error, Exception)
        if not self.error_detected:
            self.error_detected = True
            self.stop_updater()
            self.error.emit(error.errno)
            print('FrameBuffer IOError dumping frame buffer: %s' % error)

    def do_update(self, image):
        #print 'do_update %s' % QThread.currentThreadId()
        pixmap = QPixmap()
        assert isinstance(image, QImage)

        w = image.width()*self.scale
        h = image.height()*self.scale

        pixmap.convertFromImage(image.scaled(w, h, transformMode=Qt.SmoothTransformation), Qt.DiffuseDither)
        self.setPixmap(pixmap)
        # auto-adjust sizing on the pixmap frame
        self.setMinimumWidth(pixmap.width())
        self.setMinimumHeight(pixmap.height())

    def set_scale(self, scale):
        if 0 < scale < 4.0:
            self.scale = scale

    def increase_scale(self, scale):
        self.set_scale(self.scale + 0.5)

    def decrease_scale(self, scale):
        self.set_scale(self.scale - 0.5)

    def reset_scale(self):
        self.set_scale(1.0)


class ImageUpdater(QObject):
    update = pyqtSignal(QImage)
    stop = pyqtSignal()
    error = pyqtSignal(Exception)
    jpeg_error = pyqtSignal()
    buff = ''

    def __init__(self):
        QObject.__init__(self)
        self.img = QImage()

    def do_process(self):
        try:
            b = openmv.fb_dump()
        except (IOError, USBError) as e:
            print 'ImageUpdater error: %s' % e
            self.error.emit(e)
        else:
            if b:
                w = b[0]
                h = b[1]
                self.buff = b[2]
                stride = w * 3

                self.img = QImage(self.buff, w, h, stride, QImage.Format_RGB888)
                self.update.emit(self.img)

