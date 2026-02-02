QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# Мобильная сборка (Android): в Qt Creator выберите Kit "Android for arm64-v8a" и соберите проект.
android {
    QT += androidextras
    ANDROID_MIN_SDK_VERSION = 21
    ANDROID_TARGET_SDK_VERSION = 33
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    task.cpp \
    taskmanager.cpp \
    gamestats.cpp \
    englishdata.cpp

HEADERS += \
    mainwindow.h \
    task.h \
    taskmanager.h \
    gamestats.h \
    englishdata.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
