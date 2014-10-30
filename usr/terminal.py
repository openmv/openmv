
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from serial import *
from usb.core import USBError


class SerialTerminal(QPlainTextEdit):

    error = pyqtSignal()

    def __init__(self, obj):
        QPlainTextEdit.__init__(self, obj)

        self.error_detected = False

        self.reader = SerialReader()
        self.thread = QThread()
        self.reader.serial.connect(self.reader.set_serial)
        self.reader.data_ready.connect(self.handle_data)
        self.reader.error.connect(self.handle_error)

        self.timer = QTimer()
        self.timer.timeout.connect(self.reader.poll_serial)

        font = QFont('Courier', 10)
        font.setStyleHint(QFont.Monospace)
        font.setFixedPitch(True)
        self.setFont(font)

        self.setAutoFillBackground(False)
        self.setStyleSheet("background-color: black; color: lightgrey;")

    def start(self, ser):
        if ser and ser.isOpen():
            self.error_detected = False
            self.reader.serial.emit(ser)
            self.timer.start(50)

    def stop(self):
        ser = None
        self.timer.stop()

    def reset(self):
        self.clear()

    def quit(self):
        try:
            self.timer.stop()
            self.thread.quit()
            self.thread.wait(1000)
        except QErrorMessage as e:
            print('SerialTerminal error while quitting %s' % e)

    def mousePressEvent(self, mouse_event):
        ## TODO: enable select, but disable click to change cursor position
        # we don't want the mouse relocating the cursor
        if mouse_event.button() != Qt.LeftButton:
            super(SerialTerminal, self).mousePressEvent(mouse_event)

    def handle_error(self):
        if not self.error_detected:
            self.stop()
            self.error_detected = True
            self.error.emit()

    def handle_data(self, x):
        cursor = self.textCursor()
        cursor.clearSelection()
        cursor.insertText(QString(x))
        self.setTextCursor(cursor)
        return True


class SerialReader(QObject):

    serial = pyqtSignal(Serial)
    data_ready = pyqtSignal(str)
    error = pyqtSignal()

    def __init__(self):
        QThread.__init__(self)
        self.ser = None

    def set_serial(self, ser):
        self.ser = ser

    def poll_serial(self):
        if self.ser and self.ser.isOpen():
            try:
                x = ''
                while self.ser.inWaiting() > 0:
                    x += self.ser.read()
                self.data_ready.emit(x)
            except (IOError, USBError, SerialException) as e:
                self.error.emit()
                if e.errno != 5:
                    print('SerialTerminal error: %s' % e)
