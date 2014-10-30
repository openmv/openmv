
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import syntax


class PyEditor(QPlainTextEdit):
    def __init__(self, obj):
        QPlainTextEdit.__init__(self, obj)

        self.tab_stops = 4

        self.setLineWrapMode(QPlainTextEdit.NoWrap)
        font = QFont()
        font.setFamily('Courier')
        font.setStyleHint(QFont.Monospace)
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.setFont(font)
        self.setTabStopWidth(4 * QFontMetrics(font).width(' '))
        highlight = syntax.PythonHighlighter(self.document())
        self.show()

    def keyPressEvent(self, e):
        # convert tab to spaces
        if e.key() == Qt.Key_Tab:
            for x in range(self.tab_stops):
                self.insertPlainText(QString(' '))
        else:
            super(PyEditor, self).keyPressEvent(e)
