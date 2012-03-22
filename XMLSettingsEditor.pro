#-------------------------------------------------
#
# Project created by QtCreator 2012-03-01T15:28:10
#
#-------------------------------------------------

QT       += core gui

TARGET = XMLSettingsEditor
TEMPLATE = app


SOURCES += main.cpp\
        xmlsettingseditorwrapper.cpp \
    xmltreemodel.cpp

HEADERS  += xmlsettingseditorwrapper.h \
    xmltreemodel.h

FORMS    +=

OTHER_FILES += \
    icons/search.png \
    icons/save.png \
    icons/reset.png \
    icons/open.png \
    icons/forward.png \
    icons/back.png

RESOURCES += \
    icons.qrc
