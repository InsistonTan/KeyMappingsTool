QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    global.cpp \
    logwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    simulate_task.cpp

HEADERS += \
    device_info.h \
    global.h \
    key_map.h \
    logwindow.h \
    logworker.h \
    mainwindow.h \
    mapping_relation.h \
    simulate_task.h \
    utils.h

FORMS += \
    logwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../release/ -lhidapi
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../debug/ -lhidapi

# INCLUDEPATH += $$PWD/.
# DEPENDPATH += $$PWD/.

#LIBS += -L$$PWD/ -lhidapi

LIBS += -L$$PWD/ -lViGEmClient

LIBS += -ldinput8 -ldxguid

RESOURCES += \
    resource.qrc

RC_FILE = appicon.rc
