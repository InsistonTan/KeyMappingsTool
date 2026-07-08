cmake_minimum_required(VERSION 3.16)

project(KeyMappingsTool LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 防止 Windows min/max 宏污染
add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

# =========================
# 源文件
# =========================

set(SOURCES
    src/Main.cpp

    src/core/DirectInputCore.cpp
    src/models/MappingConfig.cpp
    src/models/MappingRelation.cpp
    src/services/ConfigService.cpp
    src/services/LogService.cpp
    src/services/DirectInputService.cpp

    src/ui/pages/forcefeedback_simulate_page/ForceFeedbackSimulatePage.cpp
    src/ui/pages/about_page/AboutPage.cpp
    src/ui/pages/home_page/HomePage.cpp
    src/ui/pages/log_page/LogPage.cpp
    src/ui/pages/settings_page/SettingsPage.cpp

    src/ui/widgets/CollapsibleGroupBox.cpp
    src/ui/widgets/CurveEditor.cpp
    src/ui/widgets/WinUISwitch.cpp

    src/utils/IconFactory.cpp

    src/workers/SimulateTask.cpp
    src/workers/AssistFuncWorker.cpp
    src/workers/ForceFeedbackWorker.cpp

    src/common/Global.cpp
    src/common/BtnTriggerTypeEnum.cpp

    src/models/UserConfig.cpp

    src/ui/ETS2_KeyBinder/BracketRightWidget.cpp
    src/ui/ETS2_KeyBinder/ManuallyBinder.cpp
    src/ui/ETS2_KeyBinder/ShowKeyState.cpp
    src/ui/ETS2_KeyBinder/Ets2KeyBinderWizard.cpp
    src/ui/mainwindow/MainWindow.cpp

)

set(HEADERS
    src/common/MappingConfigKey.h
    src/common/Theme.h
    src/core/DirectInputCore.h

    src/models/MappingConfig.h
    src/services/ConfigService.h
    src/services/LogService.h
    src/services/MessageBoxService.h
    src/services/DirectInputService.h

    src/ui/pages/forcefeedback_simulate_page/ForceFeedbackSimulatePage.h
    src/ui/pages/about_page/AboutPage.h
    src/ui/pages/home_page/HomePage.h
    src/ui/pages/log_page/LogPage.h
    src/ui/pages/settings_page/SettingsPage.h



    src/workers/LogWorker.h
    src/workers/AssistFuncWorker.h
    src/workers/ForceFeedbackWorker.h
    src/workers/SimulateTask.h

    src/common/DeviceDataTypeEnum.h
    src/common/StringConstants.h
    src/common/UserConfigKey.h
    src/common/BtnTriggerTypeEnum.h
    src/common/Global.h
    src/common/KeyMap.h
    src/common/BigKey.hpp

    src/models/MappingRelation.h
    src/models/UserConfig.h
    src/models/DeviceInfo.h

    src/utils/MyUtils.h
    src/utils/IconFactory.h

    src/ui/mainwindow/MainWindow.h
    src/ui/widgets/CardMessageDialog.h
    src/ui/widgets/CollapsibleGroupBox.h
    src/ui/widgets/CurveEditor.h
    src/ui/widgets/NoWheelComboBox.h
    src/ui/widgets/WinUISwitch.h
    src/ui/widgets/CustomListView.h
    src/ui/widgets/MultiSelectCombobox.h
    src/ui/ETS2_KeyBinder/BracketRightWidget.h
    src/ui/ETS2_KeyBinder/ManuallyBinder.h
    src/ui/ETS2_KeyBinder/ShowKeyState.h
    src/ui/ETS2_KeyBinder/Ets2KeyBinderWizard.h
    third_party/SCS_Telemetry/scs-telemetry-common.hpp

)

set(UI_FILES
    src/ui/ETS2_KeyBinder/ManuallyBinder.ui
    src/ui/ETS2_KeyBinder/ShowKeyState.ui
    src/ui/ETS2_KeyBinder/Ets2KeyBinderWizard.ui
)

set(RESOURCES
    resources/resource.qrc
    resources/appicon.rc
)

# =========================
# 可执行文件
# =========================

qt_add_executable(KeyMappingsTool
    WIN32
    MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}

)

# =========================
# include src third_party
# =========================
target_include_directories(KeyMappingsTool PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)


# =========================
# Qt 链接
# =========================

target_link_libraries(KeyMappingsTool PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
)

# =========================
# Windows system libs
# =========================

target_link_libraries(KeyMappingsTool PRIVATE
    dinput8
    dxguid
    user32
    winmm
)

# =========================
# 第三方库lib（ViGEm）
# =========================

target_link_directories(KeyMappingsTool PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/ViGEm
)

target_link_libraries(KeyMappingsTool PRIVATE
    ViGEmClient
)



# =========================
# Qt finalize
# =========================

qt_finalize_executable(KeyMappingsTool)
