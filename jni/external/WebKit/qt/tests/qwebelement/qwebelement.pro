TEMPLATE = app
TARGET = tst_qwebelement
include(../../../../WebKit.pri)
SOURCES  += tst_qwebelement.cpp
RESOURCES += qwebelement.qrc
QT += testlib network
QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR
