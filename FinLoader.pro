QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lstdc++fs
CONFIG += c++1z


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bar.cpp \
    configwindow.cpp \
    graph.cpp \
    importfinamform.cpp \
    main.cpp \
    mainwindow.cpp \
    marketslistmodel.cpp \
    storage.cpp \
    threadpool.cpp \
    ticker.cpp \
    tickerslistmodel.cpp

HEADERS += \
    bar.h \
    configwindow.h \
    graph.h \
    importfinamform.h \
    mainwindow.h \
    marketslistmodel.h \
    storage.h \
    threadpool.h \
    ticker.h \
    tickerslistmodel.h

FORMS += \
    configwindow.ui \
    importfinamform.ui \
    mainwindow.ui

TRANSLATIONS += \
    FinLoader_en_US.ts \
    FinLoader_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    imgres.qrc

DISTFILES +=
