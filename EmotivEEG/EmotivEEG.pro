TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -ledk -ldl
INCLUDEPATH += ./include

QMAKE_CXXFLAGS +=-std=c++1y
QMAKE_LFLAGS   +=-std=c++1y

SOURCES += \
        main.cpp \
    arguments.cpp \
    emotiveeg.cpp \
    test.cpp \
    research.cpp

HEADERS += \
    arguments.h \
    test.h \
    research.h



