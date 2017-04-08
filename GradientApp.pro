#-------------------------------------------------
#
# Project created by QtCreator 2016-07-10T22:45:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GradientApp
TEMPLATE = app

win32:RC_ICONS += gradientAppIconLarge.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    gcodeparser.cpp \
    gradientcolor.cpp

HEADERS  += mainwindow.h \
    gradientcolor.h

FORMS    += mainwindow.ui

QMAKE_LFLAGS += -static-libgcc

DISTFILES += \
    Icons/Gradient App Icon.ai \
    Icons/Gradient App Icon.ai \
    Icons/Gradient App Icon.png \
    lgpl.txt \
    README.md \
    Icons/Gradient App Icon.ai \
    Icons/Gradient App Icon.png \
    Icons/Gradient App Icon.ai \
    Icons/gradient_app_icon_01_doP_icon.ico \
    Icons/gradient_app_icon_01_gBN_icon.ico \
    Icons/gradient_app_icon_01_N04_icon.ico \
    Icons/gradient_app_icon_01_spi_icon.ico \
    Icons/Gradient App Icon.ai \
    Icons/gradientAppIconLarge.ico \
    Icons/Gradient App Icon.ai \
    gradientAppIconLarge.ico
