QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AssistFuncWindow.cpp \
    AssistFuncWorker.cpp \
    BtnTriggerTypeEnum.cpp \
    DeadAreaSettings.cpp \
    ETS2_KeyBinder/manuallybinder.cpp \
    ETS2_KeyBinder/showkeystate.cpp \
    ForceFeedbackSettingsWindow.cpp \
    ForceFeedbackWorker.cpp \
    ETS2_KeyBinder/ets2keybinderwizard.cpp \
    global.cpp \
    logwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    simulate_task.cpp

HEADERS += \
    AssistFuncWindow.h \
    AssistFuncWorker.h \
    BtnTriggerTypeEnum.h \
    DeadAreaSettings.h \
    ETS2_KeyBinder/manuallybinder.h \
    ETS2_KeyBinder/showkeystate.h \
    ForceFeedbackSettingsWindow.h \
    ForceFeedbackWorker.h \
    device_info.h \
    ETS2_KeyBinder/ets2keybinderwizard.h \
    global.h \
    key_map.h \
    logwindow.h \
    logworker.h \
    mainwindow.h \
    mapping_relation.h \
    SCS_Telemetry/scs-telemetry-common.hpp \
    simulate_task.h \
    utils.h

FORMS += \
    AssistFuncWindow.ui \
    DeadAreaSettings.ui \
    ETS2_KeyBinder/manuallybinder.ui \
    ETS2_KeyBinder/showkeystate.ui \
    ForceFeedbackSettingsWindow.ui \
    ETS2_KeyBinder/ets2keybinderwizard.ui \
    logwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../release/ -lhidapi
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../debug/ -lhidapi

INCLUDEPATH += \
    SCS_Telemetry\
    ETS2_KeyBinder

# DEPENDPATH += $$PWD/.

#LIBS += -L$$PWD/ -lhidapi

LIBS += -L$$PWD/ -lViGEmClient

LIBS += -ldinput8 -ldxguid

RESOURCES += \
    resource.qrc

RC_FILE = appicon.rc
