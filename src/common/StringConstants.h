#pragma once
#include<QString>


///
/// \brief The StringConstants class
/// 字符串常量类
///
class StringConstants{
#define qstr inline const static QString
public:
    qstr SettingsWindowTitle = QStringLiteral("设置");
    qstr SettingsWindowUnSaveTitle = QStringLiteral("设置 *改动未保存");
    qstr ffbSettingWindowTitle = QStringLiteral("力反馈模拟设置");
    qstr ffbSettingWindowUnSaveTitle = QStringLiteral("力反馈模拟设置 *设置未保存");

    qstr noString = QStringLiteral("不");
    qstr orString = QStringLiteral("或");

    qstr axisString = QStringLiteral("轴");
    qstr btnString = QStringLiteral("按键");
    qstr btnString2 = QStringLiteral("按钮");
    qstr joystick = QStringLiteral("摇杆");
    qstr angle = QStringLiteral("角度");
    qstr keyboard = QStringLiteral("键盘");
    qstr enterKey = QStringLiteral("回车");
    qstr spaceKey = QStringLiteral("空格");
    qstr action = QStringLiteral("操作");
    qstr device = QStringLiteral("设备");
    qstr refresh = QStringLiteral("刷新");
    qstr config = QStringLiteral("映射配置");
    qstr saveCurrentConfig = QStringLiteral("保存当前配置");
    qstr manageConfig = QStringLiteral("管理配置");
    qstr home = QStringLiteral("主页");
    qstr keyMapping = QStringLiteral("按键映射");
    qstr ffbSimulate = QStringLiteral("力反馈模拟");
    qstr log = QStringLiteral("日志");
    qstr configManage = QStringLiteral("配置管理");
    qstr settings = QStringLiteral("设置");
    qstr about = QStringLiteral("支持项目");
    qstr addMapping = QStringLiteral("新增映射");
    qstr btnLog = QStringLiteral("按键日志");
    qstr povLog = QStringLiteral("十字键/摇杆日志");
    qstr axisLog = QStringLiteral("轴日志");
    qstr clearLog = QStringLiteral("清除日志");
    qstr globalSettings = QStringLiteral("全局设置");
    qstr currentMappingFileSettings = QStringLiteral("当前映射配置文件专属设置");
    qstr currentMappingFileSettings2 = QStringLiteral("当前映射配置文件 \"%1\" 专属设置");
    qstr effectScope = QStringLiteral("生效范围");

    qstr brake = "刹车";
    qstr accelerator = "油门";
    qstr steeringWheelAxis = "转向轴";

    qstr text_wheelLeft = QStringLiteral("左转");
    qstr text_wheelRight = QStringLiteral("右转");

    qstr error = QStringLiteral("错误");
    qstr info = QStringLiteral("提醒");
    qstr warning = QStringLiteral("警告");
    qstr success = QStringLiteral("成功");

    qstr btnText_confirm = QStringLiteral("确认");
    qstr btnText_cancel = QStringLiteral("取消");

    qstr axisX = QStringLiteral("X轴");
    qstr axisY = QStringLiteral("Y轴");
    qstr axisZ = QStringLiteral("Z轴");
    qstr axisRX = QStringLiteral("X轴旋转");
    qstr axisRY = QStringLiteral("Y轴旋转");
    qstr axisRZ = QStringLiteral("Z轴旋转");
    qstr axisS1 = QStringLiteral("滑动轴1");
    qstr axisS2 = QStringLiteral("滑动轴2");

    qstr notSet = QStringLiteral("未设置");

    qstr btnDataLog = QStringLiteral("按键数据");
    qstr axisDataLog = QStringLiteral("轴数据");
    qstr joystickDataLog = QStringLiteral("摇杆/十字键数据");

    qstr mappingKeyBoard = QStringLiteral("映射键盘");
    qstr mappingXbox = QStringLiteral("映射Xbox");

    qstr selectedDevice = QStringLiteral("已选择设备");
    qstr selectedDeviceCount = QStringLiteral("已选择设备数");
    qstr hasNone = QStringLiteral("暂无");
    qstr versionUpdateTips = QStringLiteral("版本更新提醒");
    qstr haveNotReleaseNote = QStringLiteral("暂无更新日志");
    qstr newVersion = QStringLiteral("新版本");
    qstr releaseNote = QStringLiteral("更新内容");
    qstr lanzouDownload = QStringLiteral("蓝奏云下载");
    qstr gitHubDownload = QStringLiteral("GitHub下载");
    qstr pleaseSelectDevice = QStringLiteral("请选择设备");
    qstr noDeviceScan = QStringLiteral("未扫描到设备");

    qstr startKeyMappingsRunning = QStringLiteral("启动全局映射");
    qstr startKeyMappingsRunningSuccess = QStringLiteral("启动全局映射成功!");
    qstr startKeyMappingsRunningFailed = QStringLiteral("启动全局映射失败");
    qstr stopKeyMappingsRunning = QStringLiteral("停止全局映射");
    qstr stopKeyMappingsRunningSucess = QStringLiteral("全局映射已停止!");
    qstr currentState = QStringLiteral("当前状态");
    qstr notStart = QStringLiteral("未启动");
    qstr alreadyPause = QStringLiteral("已暂停");
    qstr alreadyStart = QStringLiteral("已启动");
    qstr blankMappings = QStringLiteral("空白配置");
    qstr saveMappings = QStringLiteral("保存配置");
    qstr lastUsedMappings = QStringLiteral("上一次使用的未保存的配置");
    qstr existsSameFileNameMappingConfig= QStringLiteral("存在同名的映射配置文件, 是否覆盖?");

    //qstr  = QStringLiteral("");
    qstr baseSettings = QStringLiteral("软件基础设置");
    qstr startOnStartup = QStringLiteral("开机自启动");
    qstr hideWindowOnClose = QStringLiteral("关闭主窗口时隐藏到系统托盘");
    qstr keyMappingSettings = QStringLiteral("按键映射相关设置");
    qstr autoMappingDesc = QStringLiteral("打开软件后自动开启映射");
    qstr autoMapping = QStringLiteral("自动开启映射");
    qstr deviceNameUniqueModeDesc = QStringLiteral("设备名称将附带设备接口路径信息");
    qstr deviceNameUniqueMode = QStringLiteral("设备名称强唯一模式");
    qstr multiKeyFirstModeDesc = QStringLiteral("优先触发最长组合键");
    qstr addMappingOnlyKeyChangeDesc = QStringLiteral("新增映射时只识别新按下的按键, 在新增映射前就一直按下的按键将被忽略");
    qstr multiKeyFirstMode = QStringLiteral("最长组合键优先模式");
    qstr addMappingOnlyKeyChange = QStringLiteral("新增映射只识别变化按键");
    qstr deadZoneSettings = QStringLiteral("死区设置");
    qstr xboxJoyStickDeadZoneDesc = QStringLiteral("数值范围:-1到1, 如果需要抵消游戏自带的死区, 请设置成负值");
    qstr xboxJoyStickDeadZone = QStringLiteral("虚拟手柄-摇杆内部死区");
    qstr xboxTriggerDeadZoneDesc = QStringLiteral("数值范围:-1到1, 如果需要抵消游戏自带的死区, 请设置成负值");
    qstr xboxTriggerDeadZone = QStringLiteral("虚拟手柄-扳机内部死区");
    qstr wheelAxisDeadZoneDesc = QStringLiteral("数值范围:0到1, 值越大, 需要越多的转向才能触发映射的按键");
    qstr wheelAxisDeadZone = QStringLiteral("设备轴映射按键-转向轴内部死区");
    qstr pedalAxisDeadZoneDesc = QStringLiteral("数值范围:0到1, 值越大, 需要踩下越深的踏板才能触发映射的按键");
    qstr pedalAxisDeadZone = QStringLiteral("设备轴映射按键-踏板轴内部死区");
    qstr ETS2_settings = QStringLiteral("欧卡2相关设置");
    qstr selectDir = QStringLiteral("选择目录");
    qstr bindingGuide = QStringLiteral("绑定向导");
    qstr ETS2_installDir = QStringLiteral("欧卡2安装目录");
    qstr autoReleaseHandBrakeDesc = QStringLiteral("需要先选择欧卡2安装目录, 然后游戏内手刹的键盘按键设置成'空格'");
    qstr autoReleaseHandBrake = QStringLiteral("欧卡2自动解除手刹");
    qstr ETS2_keyBinderDesc = QStringLiteral("通过修改游戏配置, 支持绑定: 近光灯, 转向灯, 雨刷等等");
    qstr ETS2_keyBinder = QStringLiteral("欧卡2/美卡-原生按键绑定");
    qstr otherSettings = QStringLiteral("其他设置");
    qstr mouseSpeedDesc = QStringLiteral("数值范围:大于0, 值越大, 模拟鼠标移动越快");
    qstr mouseSpeed = QStringLiteral("鼠标移动速率");
    qstr notSelectedDir = QStringLiteral("还未选择目录");

    qstr deviceName = QStringLiteral("设备");
    qstr deviceButton = QStringLiteral("设备按键");
    qstr mapTo = QStringLiteral("映射成");
    qstr mappingMode = QStringLiteral("映射模式");
    qstr mappingKey = QStringLiteral("映射按键");
    qstr keyTriggerMode = QStringLiteral("按键触发模式");
    qstr remark = QStringLiteral("备注");
    qstr deleteMappings = QStringLiteral("删除该映射");
    qstr rotateAxis = QStringLiteral("反转该轴");
    qstr rotateAxisToolTips = QStringLiteral("踏板轴将自动识别是否需要反转该轴");

    qstr selectETS2FolderTitle = QStringLiteral("选择欧卡2文件夹");

    qstr pleaseEnterMappingsName = QStringLiteral("请输入配置名称:");
    qstr saveMappingsSuccess = QStringLiteral("配置保存成功!");
    qstr mappingsNameNotAllowEmpty = QStringLiteral("配置名称不能为空!");

    qstr text_pleaseConfirm = QStringLiteral("请确认");
    qstr text_confirmAxisMappingMessageBoxText = QStringLiteral("检测到设备 \"%1\" 的轴 \"%2\" \n\n请确认该轴是 映射键盘按键 还是 映射xbox手柄");
    qstr text_confirmAxisTypeMessageBoxText = QStringLiteral("轴映射键盘按键需要区分转向轴和踏板轴\n\n请确认是转向轴, 还是踏板轴");

    qstr btnText_isWheelAxis = QStringLiteral("是转向轴");
    qstr btnText_isPedalAxis = QStringLiteral("是踏板轴");
    qstr btnText_mappingKeyboard = QStringLiteral("映射键盘按键");
    qstr btnText_mappingXbox = QStringLiteral("映射xbox手柄");

    qstr btnText_addNewMapping_default = QStringLiteral("新增映射");
    qstr btnText_addNewMapping_waitingDeviceInput = QStringLiteral("等待输入...");

    qstr error_deviceNotSelect = QStringLiteral("当前已选择的设备为空, 请先选择设备");
    qstr error_pleaseStopKeyMappingRunning = QStringLiteral("请先停止全局映射!");
    qstr error_alreadyExistsMapping = QStringLiteral("设备 \"%1\" 的 \"%2\" 已经配置了映射!");
    qstr error_leftOrRightAlreadyExistsMapping = QStringLiteral("设备 \"%1\" 的 \"%2左转\" 或 \"%2右转\" 已经配置了映射! \n\n 不能再重复配置!");
    qstr error_mappingsListIsEmpty = QStringLiteral("映射列表为空!");
    qstr error_btnNotPressed = QStringLiteral("未检测到按键和轴的值发生变化, 请重试!");

    qstr openETS2ShareMemoryFailed = QStringLiteral("无法打开共享内存:%1, 欧卡2可能未运行, 将等待欧卡2运行...");
    qstr openETS2ShareMemorySuccess = QStringLiteral("打开共享内存:%1 成功!");
    qstr readDataFromETS2ShareMemoryFailed = QStringLiteral("从共享内存获取数据失败!");
    qstr readDataFromETS2ShareMemorySuccess = QStringLiteral("从共享内存获取数据成功!");
    qstr releasingHandBrake = QStringLiteral("当前手刹为启用状态, 且油门大于50%, 正在模拟空格键解除手刹...");

    qstr saveSettingsFailed = QStringLiteral("保存设置失败");
    qstr openSettingsFileFailed = QStringLiteral("打开/创建设置文件失败!");
    qstr parseJsonFailed = QStringLiteral("解析设置文件的json内容失败!");

    qstr enableStartOnStartup = QStringLiteral("已设置开机自启动");
    qstr disableStartOnStartup = QStringLiteral("已取消开机自启动");

    qstr devicesNotMatchTips = QStringLiteral("当前映射配置需要以下设备:\n%1\n\n但当前没有选择这些设备, 属于这些设备的按键映射将不会有响应");

    qstr enableETS2AutoReleaseHandBrake = QStringLiteral("<b style='color:rgb(0, 151, 144);'>开启</b> 欧卡2自动解除手刹");
    qstr disableETS2AutoReleaseHandBrake = QStringLiteral("<b style='color:red;'>关闭</b> 欧卡2自动解除手刹");

    qstr enableAutoStartMappings = QStringLiteral("<b style='color:rgb(0, 151, 144);'>开启</b> 打开软件后立即开启映射");
    qstr disableAutoStartMappings = QStringLiteral("<b style='color:red;'>关闭</b> 打开软件后立即开启映射");

    qstr enableCombineBtnFirstMode = QStringLiteral("<b style='color:rgb(0, 151, 144);'>开启</b> 最长组合键优先模式");
    qstr disableCombineBtnFirstMode = QStringLiteral("<b style='color:red;'>关闭</b> 最长组合键优先模式");

    qstr enableOnlyChangedKey = QStringLiteral("<b style='color:rgb(0, 151, 144);'>开启</b> 新增映射时只返回变化的按键");
    qstr disableOnlyChangedKey = QStringLiteral("<b style='color:red;'>关闭</b> 新增映射时只返回变化的按键");

    qstr enableFFBSimulate = QStringLiteral("<b style='color:rgb(0, 151, 144);'>开启</b> 方向盘力反馈模拟");
    qstr disableFFBSimulate = QStringLiteral("<b style='color:red;'>关闭</b> 方向盘力反馈模拟");

    qstr lastMappingModeFileNotExists = QStringLiteral("记录上一次使用的设备的缓存文件不存在!");
    qstr loadLastDeviceSuccess = QStringLiteral("读取上一次使用的设备成功!");
    qstr loadLastDeviceFailed = QStringLiteral("读取上一次使用的设备失败: 配置文件无法打开!");
    qstr lastDeviceInCurrentDeviceList = QStringLiteral("上一次使用的设备 \"%1\" 在当前设备列表中, 自动选择该设备");
    qstr lastDeviceNotInCurrentDeviceList = QStringLiteral("上一次使用的设备 \"%1\" 不在当前设备列表中, 跳过该设备");
    qstr openFileFailed = QStringLiteral("打开文件 \"%1\" 失败!");
    qstr ets2FolderNotExists = QStringLiteral("文件夹不存在: %1 ,请确保欧卡2的安装目录正确!");
    qstr checkLatestVersionFailed = QStringLiteral("<b>检查版本更新</b>: api  \"%1\"  访问失败, 错误信息: %2");

    qstr deviceStatusDataLog = QStringLiteral(" \"%1\" 设备状态数据:<br/>");

    qstr initDirectInputErrorMsg = QStringLiteral("初始化DirectInput: 初始化失败！");
    qstr initDirectInputSuccessMsg = QStringLiteral("DirectInput 初始化成功！");
    qstr scanDeviceErrorMsg = QStringLiteral("扫描设备列表失败！");
    qstr scanDeviceSuccessMsg = QStringLiteral("扫描设备成功！");

    qstr deviceNotFoundErrorMsg = QStringLiteral("初始化设备  \"%1\"  失败: 该设备不在设备列表中, 请手动刷新后重试");
    qstr initDeviceUnknownError = QStringLiteral("初始化设备  \"%1\"  失败: 未知错误！");
    qstr createDeviceErrorMsg = QStringLiteral("初始化设备  \"%1\"  失败: 设备创建失败！");
    qstr setDataFormatErrorMsg = QStringLiteral("初始化设备  \"%1\"  失败: 设置数据格式失败！");
    qstr setCooperativeLevelErrorMsg = QStringLiteral("初始化设备  \"%1\"  失败: 设置协作模式失败！");
    qstr getCapabilitiesErrorMsg = QStringLiteral("初始化设备  \"%1\"  失败: 获取设备能力失败！");
    qstr connectDeviceSuccessMsg = QStringLiteral("连接设备 \"%1\" 成功！");
    qstr getAxisValueRangeErrorMsg = QStringLiteral("获取设备 \"%1\" 的数值范围失败, 如果该设备没有此轴, 请忽略此警告");
    qstr notSupportFFBErrorMsg = QStringLiteral("设备 \"%1\" 不支持力反馈!");
    qstr checkSupportFFBErrorMsg = QStringLiteral("检测设备是否支持力反馈失败: 初始化设备失败!");
    qstr getJoyStateErrorMsg = QStringLiteral("获取设备 \"%1\" 的状态数据失败!");

    qstr startETS2AutoHandBrakeError = QStringLiteral("欧卡2自动解除手刹功能启动失败! \n\n详情请看日志");
    qstr eTS2PathIsEmptyError = QStringLiteral("欧卡2安装路径为空!");
    qstr copyPluginWarning = QStringLiteral("欧卡2目录未检测到遥测数据插件 \"%1\" , 将复制插件到欧卡2 plugins 目录 \"%2\" ...");
    qstr copyPluginFolderNotExistsError = QStringLiteral("%1 不存在, 无法将该插件复制到欧卡2 plugins 目录!");
    qstr copyPluginFailed = QStringLiteral("复制插件失败!");
    qstr copyPluginSuccess = QStringLiteral("复制插件成功, 重启游戏后生效");
    qstr readSteamPathFailed = QStringLiteral("从注册表读取steam安装路径失败, 请在 首页-辅助功能-选择欧卡目录 手动选择欧卡安装目录!");
    qstr readSteamPathSuccess = QStringLiteral("读取steam目录成功: %1");
    qstr readETS2PathFromSteamPthFailed = QStringLiteral("从steam安装目录读取欧卡2安装目录失败, 请在 首页-辅助功能-选择欧卡目录 手动选择欧卡安装目录!");
    qstr readETS2PathSuccess = QStringLiteral("读取欧卡2安装目录成功: %1");

    qstr openAssistFunctionConfigFailed = QStringLiteral("打开辅助功能设置文件失败!");
    qstr readAssistFunctionConfigFailed = QStringLiteral("解析辅助功能设置文件的json内容失败!");

    qstr getInitedDeviceByDeviceNameFailed = QStringLiteral("获取已初始化的设备实例失败, 设备: %1");

    qstr ffbsettingWindow_BtnText_steeringWheel_waitingForRotate = QStringLiteral("等待方向盘转动...");
    //qstr ffbsettingWindow_BtnText_steeringWheel_default = QStringLiteral("点击后转动方向盘");
    qstr ffbsettingWindow_BtnText_thottle_waitingForRotate = QStringLiteral("等待油门踩下...");
    //qstr ffbsettingWindow_BtnText_thottle_default = QStringLiteral("点击后踩下油门");
    qstr ffbsettingWindow_BtnText_brake_waitingForRotate = QStringLiteral("等待刹车踩下...");
    //qstr ffbsettingWindow_BtnText_brake_default = QStringLiteral("点击后踩下刹车");
    qstr ffbsettingWindow_Error_steeringWheelNotConnect = QStringLiteral("未检测到方向盘转动, 将重置该项! 请检查设备与电脑的连接是否正常.");
    qstr ffbsettingWindow_Error_thottleNotConnect = QStringLiteral("未检测到油门踏板踩下, 将重置该项! 请检查设备与电脑的连接是否正常.");
    qstr ffbsettingWindow_Error_brakeNotConnect = QStringLiteral("未检测到刹车踏板踩下, 将重置该项! 请检查设备与电脑的连接是否正常.");

    qstr ffbSimulateThread = QStringLiteral("力反馈模拟线程");
    qstr ffbSimulateThread_successMsg = QStringLiteral("开启力反馈模拟成功!");
    qstr ffbSimulateThread_finishedMsg = QStringLiteral("力反馈模拟已关闭");
    qstr ffbSimulateThread_WheelDeviceEmptyErrorMsg = QStringLiteral("力反馈模拟: 转向轴设备为空, 请设置或重新设置转向轴设备.");
    qstr ffbSimulateThread_WheelDeviceOpenErrorMsg = QStringLiteral("力反馈模拟: 转向轴设备初始化/连接失败, 请检查该设备与电脑的连接是否正常.");
    qstr ffbSimulateThread_throttleOrBrakeDeviceEmptyErrorMsg = QStringLiteral("力反馈模拟: 油门轴设备/刹车轴设备为空, 请设置或重新设置.");
    qstr ffbSimulateThread_throttleOrBrakeDeviceOpenErrorMsg = QStringLiteral("力反馈模拟: 油门轴设备/刹车轴设备连接失败, 请检查该设备与电脑的连接是否正常.");
    qstr ffbSimulateThread_throttleOrBrakeDeviceGetInstanceErrorMsg = QStringLiteral("力反馈模拟: 获取已初始化的油门轴设备/刹车轴设备实例失败!");
    qstr ffbSimulateThread_steeringAxisRangeErrorMsg = QStringLiteral("开启力反馈模拟失败: 获取转向轴数值范围失败!");
    qstr ffbSimulateThread_throttleAxisRangeErrorMsg = QStringLiteral("开启力反馈模拟失败: 获取油门踏板数值范围失败!");
    qstr ffbSimulateThread_brakeAxisRangeErrorMsg = QStringLiteral("开启力反馈模拟失败: 获取刹车踏板数值范围失败!");
    qstr ffbSimulateThread_steeringWheelDeviceErrorMsg = QStringLiteral("开启力反馈模拟失败: 当前设置的转向设备 \"%1\" 不支持力反馈");
    qstr ffbSimulateThread_multiDeviceErrorMsg = QStringLiteral("开启力反馈模拟失败: \n\n当前选择了多个设备, 但是目前的配置是旧版单设备的配置\n\n请选择单个设备 或者 重新配置一下转向轴,油门轴和刹车轴");
    qstr ffbSimulateThread_deviceNotSettingErrorMsg = QStringLiteral("开启力反馈模拟失败: 未设置转向轴/油门轴/刹车轴的设备!");
    qstr ffbSimulateThread_deviceSelectedEmptyErrorMsg = QStringLiteral("开启力反馈模拟失败: 还未选择设备!");
    qstr ffbSimulateThread_createEffectsErrorMsg = QStringLiteral("开启力反馈模拟失败: 创建力反馈效果失败!");
    qstr ffbSimulateThread_initDevicesErrorMsg = QStringLiteral("开启力反馈模拟失败: 初始化设备/打开设备失败!");

    qstr error_globalControlsFileError = QStringLiteral("\n\n文件路径：\n%1\n\n请返回第一步，确定已禁用Steam输入");
    qstr error_globalControlsFileNotExists = QStringLiteral("游戏全局配置文件不存在！");
    qstr error_globalControlsFileOpenFailed = QStringLiteral("打开游戏全局配置文件失败！");
    qstr error_gameProfileNotExists = QStringLiteral("游戏配置文件不存在！文件路径：\n%1");
    qstr error_gameProfileOpenFailed = QStringLiteral("打开游戏配置文件失败！文件路径：\n%1");
    qstr error_gameProfileWriteFailed = QStringLiteral("无法写入游戏配置文件！文件路径：\n%1");
    qstr error_GameFileNotExists = QStringLiteral("该文件不存在！参考路径：\n\n%1/xxxxxx/controls.sii\n%2/xxxxxx/controls.sii\n");
    qstr error_backupProfileFailed = QStringLiteral("备份游戏配置文件失败：");
    qstr error_writePrifileFailed = QStringLiteral("修改游戏配置文件失败：");
    qstr error_gameProfileFolderNotExists = QStringLiteral("没有找到任何游戏配置文件夹");
    qstr error_changedBtnNotFound = QStringLiteral("没有找到变化的按键！");
    qstr error_foundMultiBtnPressed = QStringLiteral("找到多个按键按下！请重新操作！");
    qstr error_notSelectDevice = QStringLiteral("还未选择游戏输入设备, 请返回第三步选择");
    qstr error_someActionNotFoundChangedBtn = QStringLiteral("部分操作没有找到变化的按键！");
    qstr error_notSupportThisHighbeam = QStringLiteral("游戏不支持开关类型的远光灯绑定\n\n请回到主界面\n选择设备：%1\n选择映射：“映射键盘”\n配置文件：\"%2\"");
    qstr error_matchGameProfileFailed = QStringLiteral("自动匹配 游戏输入类型和游戏配置文件 失败!       \n\n请手动选择");
    qstr error_deviceConnectErrorMsg = QStringLiteral("请返回第3步，将设备连接到电脑，\n在“硬件控制设备”下拉框选择设备\n%1\n或者进行手动绑定？");
    qstr error_thisAxisAlreadySet = QStringLiteral("该轴已被设置为%1!");
    qstr error_loadConfigFileFailed_getPathFailed = QStringLiteral("加载配置文件 \"%1\" 失败:\n\n获取该文件绝对路径失败!");
    qstr error_writeMappingFileFailed = QStringLiteral("打开/写入文件失败: \n%1");
    qstr error_mappingFilenameEmpty = QStringLiteral("映射配置文件名为空, 无法加载!");
    qstr error_parseMappingFileFailed = QStringLiteral("读取映射配置文件失败, 该文件可能已损坏");
    qstr error_deleteRowFailed = QStringLiteral("删除该映射失败!");

    qstr deviceConnectError = QStringLiteral("设备连接异常");
    qstr matchGameProfileFailed = QStringLiteral("匹配失败");
    qstr matchGameProfileSuccess = QStringLiteral("匹配成功");
    qstr matchGameProfileSuccessMsg = QStringLiteral("自动匹配 游戏输入类型和游戏配置文件 成功! \n\n游戏输入类型: %1\n\n游戏配置文件: %2");
    qstr wiringTipWiper = QStringLiteral("接线提示-五菱宏光-雨刮拨杆");
    qstr wiringTipTurnSignalStalk = QStringLiteral("接线提示-五菱宏光-转向灯拨杆");
    qstr toggle = QStringLiteral("拨到");
    qstr lowLevel = QStringLiteral("低档位");
    qstr highLevel = QStringLiteral("高档位");
    qstr gearSwitch1 = QStringLiteral("档位开关1");
    qstr gearSwitch2 = QStringLiteral("档位开关2");
    qstr please = QStringLiteral("请将");
    qstr level3 = QStringLiteral("3档");
    qstr level4 = QStringLiteral("4档");
    qstr wantToBindWhichKey = QStringLiteral("您要绑定 %1 还是 %2");
    qstr wiperLevel1 = QStringLiteral("雨刮器1档");
    qstr wiperLevel2 = QStringLiteral("雨刮器2档");
    qstr wiperLevel3 = QStringLiteral("雨刮器3档");
    qstr wiperLevel4 = QStringLiteral("雨刮器4档（点动）");
    qstr closeWiper = QStringLiteral("关闭雨刷");
    qstr wiper = QStringLiteral("雨刮器");
    qstr level3Wiper = QStringLiteral("3档雨刮器");
    qstr level4Wiper = QStringLiteral("4档雨刮器(3档+点动)");
    qstr leftAndRightSignal = QStringLiteral("左转向灯&右转向灯");
    qstr leftTurnSignal = QStringLiteral("左转向灯");
    qstr rightTurnSignal = QStringLiteral("右转向灯");
    qstr whichKeyToBindingYouWant = QStringLiteral("\n您是想绑定：");
    qstr highbeamAndHeadlightFlashHasSameKey = QStringLiteral("远光灯和灯光喇叭的按键相同\n");
    qstr headlightFlash = QStringLiteral("灯光喇叭");
    qstr highbeam = QStringLiteral("远光灯");
    qstr highbeamAndHeadlightFlash = QStringLiteral("远光灯&灯光喇叭");
    qstr positionLights = QStringLiteral("示廓灯");
    qstr lowbeam = QStringLiteral("近光灯");
    qstr closeLight = QStringLiteral("关闭灯光");
    qstr closePosition = QStringLiteral("关闭位置");
    qstr toggleStalk = QStringLiteral("请将拨杆拧到");
    qstr toggleStalk2 = QStringLiteral("请将拨杆拨到");
    qstr positionLightsAndLowbeam = QStringLiteral("示廓灯&近光灯");
    qstr confirmBinding = QStringLiteral("是否绑定？");
    qstr singleActionBinding = QStringLiteral("单操作绑定");
    qstr multiActionBinding = QStringLiteral("多操作绑定");

    qstr installXboxDriverTips = QStringLiteral("检测到虚拟手柄驱动未安装, 驱动安装包所在位置:\n%1\n\n点击确认将运行驱动安装包");
    qstr xboxDriverFileNotExistsTips = QStringLiteral("虚拟手柄驱动的安装包不存在:\n%1");

    qstr ets2AssistFuncWorkerFinished = QStringLiteral("欧卡2辅助功能线程结束!");
    qstr ets2AssistFuncWorkerStart = QStringLiteral("欧卡2辅助功能线程正在运行...");

    qstr setBrakeAxisBtn = QStringLiteral("点击后踩下刹车");
    qstr brakeAxis = QStringLiteral("刹车");
    qstr throttleAxis = QStringLiteral("油门");
    qstr setThrottleAxisBtn = QStringLiteral("点击后踩下油门");
    qstr steeringAxis = QStringLiteral("转向(力反馈轴)");
    qstr setSteringAxisBtn = QStringLiteral("点击后转动方向盘");
    qstr axisSettings = QStringLiteral("设置转向/油门/刹车");
    qstr titleSpeed = QStringLiteral("车速百分比");
    qstr titleSpringGain = QStringLiteral("回正力强度系数百分比");
    qstr titleDampingGain = QStringLiteral("转向阻尼强度系数百分比");
    qstr carParamsSettings = QStringLiteral("设置游戏内车辆参数");
    qstr gainSettings = QStringLiteral("设置力反馈最大强度");
    qstr springMaxGain = QStringLiteral("回正力最大强度");
    qstr springMaxGainDesc = QStringLiteral("数值范围: 0到1");
    qstr damperMaxGain = QStringLiteral("转向阻尼最大强度");
    qstr sdamperMaxGainDesc = QStringLiteral("数值范围: 0到1");
    qstr speedUp100km_h = QStringLiteral("百公里加速(单位:秒)");
    qstr speedDown100km_h = QStringLiteral("百公里刹停(单位:米)");
    qstr maxSpeed = QStringLiteral("最高时速(单位:km/h)");
    qstr ffbCurveSettings = QStringLiteral("自定义力反馈动态曲线");
    qstr ffbSimSwitchText = QStringLiteral("开启/关闭 力反馈模拟");
    qstr speedUpDesc = QStringLiteral("数值范围: 大于0");
    qstr speedDownDesc = QStringLiteral("数值范围: 大于0");
    qstr maxSpeedDesc = QStringLiteral("数值范围: 大于0");
    qstr ffbCurveActionDesc = QStringLiteral("操作说明:\n添加点: 双击鼠标左键\n删除点: 单击鼠标右键\n移动点: 长按鼠标左键\n调整曲线曲率: 移动白色点");
    qstr reverseAxisTooltip = QStringLiteral("自动识别是否需要反转, 一般情况下不需要手动操作此项");
    qstr needSaveMappingFileTip = QStringLiteral("当前映射配置还未保存到本地, 请先保存才能进行此操作");
    qstr autoSaveMappingFileFailed = QStringLiteral("自动保存失败, 请先手动保存一次当前映射配置");
    qstr invalidValue = QStringLiteral("数值无效，请输入数值范围内的有效数值");
    qstr overrideGlobalSettings = QStringLiteral("覆盖全局设置");
    qstr overrideGlobalSettingsDesc = QStringLiteral("只有开启此选项, 才会使用专属设置, 否则会默认使用全局设置");

    qstr appError = QStringLiteral("程序错误");
    qstr appException = QStringLiteral("程序异常");
    qstr appUnhandedException = QStringLiteral("程序出现未处理异常: \n%1");
    qstr appUnhandedUnknownException = QStringLiteral("程序出现未知类型的未处理异常");

    qstr supportTitle = QStringLiteral("支持项目 ❤️");
    qstr supportIntro = QStringLiteral("感谢你的使用！\n如果觉得这个项目对你有帮助, 或者觉得有意思, 可以给这个项目一个支持。\n你的支持会帮助项目持续改进。");
    qstr supportFree = QStringLiteral("⭐ 免费支持");
    qstr supportFreeIntro = QStringLiteral("给项目一个 Star\n\n一个 Star 是对开发者最大的鼓励，也能帮助更多用户发现这个项目。\n");
    qstr githubProjectBtn = QStringLiteral("GitHub项目主页");
    qstr donate = QStringLiteral("☕ 赞赏支持");
    qstr donateIntro = QStringLiteral("如果你愿意进一步支持项目开发，可以给予一次性赞赏。\n\n所有赞赏都将用于项目的持续开发和维护。\n");

    qstr showMainWindow = QStringLiteral("显示窗口");
    qstr quit = QStringLiteral("退出");
};
