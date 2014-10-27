
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from serial import *


class SerialTerminal(QPlainTextEdit):
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
        self.timer.stop()

    def reset(self):
        self.clear()

    def quit(self):
        try:
            self.timer.stop()
            self.thread.quit()
            self.thread.wait(1000)
        except QErrorMessage as e:
            print('error quitting reader %s' % e.message)

    def mousePressEvent(self, mouse_event):
        ## TODO: enable select, but disable click to change cursor position
        # we don't want the mouse relocating the cursor
        if mouse_event.button() != Qt.LeftButton:
            super(SerialTerminal, self).mousePressEvent(mouse_event)

    def handle_error(self, e):
        if not self.error_detected:
            self.error_detected = True

    def handle_data(self, x):
        cursor = self.textCursor()
        cursor.clearSelection()
        cursor.insertText(QString(x))
        self.setTextCursor(cursor)
        return True


class SerialReader(QObject):

    serial = pyqtSignal(Serial)
    data_ready = pyqtSignal(str)
    error = pyqtSignal(str)

    def __init__(self):
        QThread.__init__(self)
        print('init serial reader')
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
            except IOError as e:
                print('serial error: %s' % e)
                self.error.emit(e.message)