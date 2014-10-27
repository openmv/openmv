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
        self.updater.error.connect(self.do_error)
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

    def do_error(self, error):
        assert isinstance(Exception)
        if not self.error_detected:
            self.error_detected = True
            self.stop_updater()
            self.error.emit(error)
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

    def increase_scale(self, scale):
        if self.scale < 4.0:
            self.scale += 0.5

    def decrease_scale(self, scale):
        if self.scale > 0.5:
            self.scale -= 0.5

    def reset_scale(self):
        self.scale = 1.0


class ImageUpdater(QObject):
    update = pyqtSignal(QImage)
    stop = pyqtSignal()
    error = pyqtSignal(str)
    jpeg_error = pyqtSignal()

    def __init__(self):
        QObject.__init__(self)
        self.image = QImage()

    def do_process(self):
        try:
            b = openmv.fb_get()
        except (IOError, USBError) as e:
            print 'ImageUpdater IOError %s' % e
            self.error.emit(e.errno)
        else:
            if b:
                fmt = b[0]
                w = b[1]
                h = b[2]
                buff = b[3]

                img = None

                if fmt == openmv.FORMAT_JPEG:
                    ## JPEG decoding required
                    img = QImage(w, h, QImage.Format_ARGB32)
                    buff = string.join(map(lambda z: '%c' % z, buff), '')
                    if not img.loadFromData(buff, 'JPG'):
                        print('ImageUpdater JPEG decode error')
                        self.jpeg_error.emit()

                elif fmt == openmv.FORMAT_GRAY:
                    ## We've got a grayscale image
                    img = QImage(w, h, QImage.Format_ARGB32)
                    x = 0
                    y = 0
                    for c in buff:
                        img.setPixel(x, y, qRgb(c, c, c))
                        x += 1
                        if x >= w:
                            y += 1
                            x = 0
                elif fmt == openmv.FORMAT_RGB565:
                    ## We've got RGB565 format
                    buff = numpy.frombuffer(buff, dtype=numpy.uint16).byteswap()
                    img = QImage(buff, w, h, QImage.Format_RGB16)
                else:
                    print('ImageUpdater unknown image format')

                if img:
                    self.image = img
                    self.update.emit(self.image)

