QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QMAKE_CXXFLAGS += -std=gnu++0x -pthread
#QMAKE_CFLAGS += -std=gnu++0x -pthread

LIBS += -lstdc++fs
LIBS += -pthread
CONFIG += c++1z



# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bar.cpp \
    bargraphicsitem.cpp \
    bartick.cpp \
    blockfreequeue.cpp \
    bulbululator.cpp \
    configwindow.cpp \
    databuckgroundthreadanswer.cpp \
    datafinloadtask.cpp \
    datafinquotesparse.cpp \
    graph.cpp \
    graphholder.cpp \
    graphviewform.cpp \
    importfinqotesform.cpp \
    main.cpp \
    mainwindow.cpp \
    market.cpp \
    modelmarketslist.cpp \
    modeltickerslist.cpp \
    plusbutton.cpp \
    storage.cpp \
    styledswitcher.cpp \
    threadfreecout.cpp \
    threadfreelocaltime.cpp \
    threadpool.cpp \
    ticker.cpp \
    workerloader.cpp

HEADERS += \
    bar.h \
    bargraphicsitem.h \
    bartick.h \
    blockfreequeue.h \
    bulbululator.h \
    configwindow.h \
    databuckgroundthreadanswer.h \
    datafinloadtask.h \
    datafinquotesparse.h \
    graph.h \
    graphholder.h \
    graphviewform.h \
    importfinqotesform.h \
    mainwindow.h \
    market.h \
    modelmarketslist.h \
    modeltickerslist.h \
    plusbutton.h \
    storage.h \
    styledswitcher.h \
    threadfreecout.h \
    threadfreelocaltime.h \
    threadpool.h \
    ticker.h \
    workerloader.h

FORMS += \
    configwindow.ui \
    graphviewform.ui \
    importfinqotesform.ui \
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
