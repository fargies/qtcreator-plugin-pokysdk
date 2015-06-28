include(../../qtcreatorplugin.pri)

HEADERS = pokysdk.h \
          pokysdkplugin.h \
    pokyrunner.hpp \
    pokysdkkitinformation.h

SOURCES = pokysdk.cpp \
          pokysdkplugin.cpp \
    pokyrunner.cpp \
    pokysdkkitinformation.cpp

DEFINES += POKYSDK_LIBRARY

RESOURCES += \
    pokysdk.qrc
