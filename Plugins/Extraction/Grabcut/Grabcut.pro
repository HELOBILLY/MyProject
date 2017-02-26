QT       += core gui xml

TARGET = Grabcut
TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../../bin/Plugins

OTHER_FILES += Grabcut.json

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix{
LIBS += -lgdal
QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH += /usr/include/gdal \
/usr/local/include/gdal \
/usr/include/qgis \
/usr/local/include/qgis

DEFINES += CORE_EXPORT=
DEFINES += GUI_EXPORT=
}
!unix{
#QGIS
INCLUDEPATH += $(QGIS_ROOT)/include \
                $(OSGEO4W_ROOT)/include
LIBS += -L$(QGIS_ROOT)/lib \
                -L$(OSGEO4W_ROOT)/lib
DEFINES += CORE_EXPORT=__declspec(dllimport)
DEFINES += GUI_EXPORT=__declspec(dllimport)

LIBS += -lgdal_i
}

LIBS += -lqgis_core -lqgis_gui

HEADERS += \
    cdtgrabcutmaptool.h \
    grabcutinterface.h \
    grabcut_opencv.h

SOURCES += \
    cdtgrabcutmaptool.cpp \
    grabcutinterface.cpp \
    grabcut_opencv.cpp

include(../../../Interfaces/Interfaces.pri)
include(../../../Tools/Config/link2opencv.pri)
