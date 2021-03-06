QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# TODO: delete
#QMAKE_CXXFLAGS += -std=gnu++0x -pthread
#QMAKE_CFLAGS += -std=gnu++0x -pthread

# linking is needed only at unix
unix: LIBS += -lstdc++fs
unix: LIBS += -pthread
CONFIG += c++1z



# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutform.cpp \
    amipipeholder.cpp \
    amipipesform.cpp \
    bar.cpp \
    bargraphicsitem.cpp \
    bartick.cpp \
    blockfreequeue.cpp \
    bulbululator.cpp \
    combindicator.cpp \
    configwindow.cpp \
    dataamipipeanswer.cpp \
    dataamipipetask.cpp \
    databuckgroundthreadanswer.cpp \
    datafastloadtask.cpp \
    datafinloadtask.cpp \
    datafinquotesparse.cpp \
    dateitemdelegate.cpp \
    fasttasksholder.cpp \
    graph.cpp \
    graphholder.cpp \
    graphviewform.cpp \
    importfinqotesform.cpp \
    main.cpp \
    mainwindow.cpp \
    market.cpp \
    memometer.cpp \
    modelmarketslist.cpp \
    modelsessions.cpp \
    modeltickerslist.cpp \
    oneofkindprotector.cpp \
    plusbutton.cpp \
    splashwindow.cpp \
    storage.cpp \
    styledswitcher.cpp \
    threadfreecout.cpp \
    threadfreelocaltime.cpp \
    threadpool.cpp \
    ticker.cpp \
    transparentbutton.cpp \
    utilites.cpp \
    win32namedpipe.cpp \
    workerloader.cpp

HEADERS += \
    aboutform.h \
    amipipeholder.h \
    amipipesform.h \
    bar.h \
    bargraphicsitem.h \
    bartick.h \
    blockfreequeue.h \
    bulbululator.h \
    combindicator.h \
    configwindow.h \
    dataamipipeanswer.h \
    dataamipipetask.h \
    databuckgroundthreadanswer.h \
    datafastloadtask.h \
    datafinloadtask.h \
    datafinquotesparse.h \
    dateitemdelegate.h \
    fasttasksholder.h \
    graph.h \
    graphholder.h \
    graphviewform.h \
    importfinqotesform.h \
    mainwindow.h \
    market.h \
    memometer.h \
    modelmarketslist.h \
    modelsessions.h \
    modeltickerslist.h \
    oneofkindprotector.h \
    plusbutton.h \
    splashwindow.h \
    storage.h \
    styledswitcher.h \
    threadfreecout.h \
    threadfreelocaltime.h \
    threadpool.h \
    ticker.h \
    transparentbutton.h \
    utilites.h \
    win32namedpipe.h \
    workerloader.h

FORMS += \
    aboutform.ui \
    amipipesform.ui \
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
