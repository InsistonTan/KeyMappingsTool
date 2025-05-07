#include "ets2keybinderwizard.h"
#include "manuallybinder.h"
#include "ui_ets2keybinderwizard.h"
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <map>
#include <string>

// 参考开源项目：https://github.com/Sab1e-GitHub/ETS2-KeyBinder
// 参考开源项目：https://github.com/InsistonTan/KeyMappingsTool

const QString MEG_BOX_LINE = "------------------------";
const QString MAPPING_FILE_NAME = "LightBinder";

using namespace std;

// 欧卡2 设置    “摇杆 Button0” 0基索引
// 欧卡2 配置文件 “joy.b1”      1基索引
// dinput Button_Index         0基索引

ETS2KeyBinderWizard::ETS2KeyBinderWizard(QWidget* parent) : QWizard(parent), ui(new Ui::ETS2KeyBinderWizard) {
    ui->setupUi(this);
#if defined(INDEPENDENT_MODE)
    this->setWindowTitle("欧卡2/美卡-特殊按键绑定向导（实验性）" + QString(__DATE__) + " " + QString(__TIME__));
#else
    this->setWindowTitle("欧卡2/美卡-特殊按键绑定向导（实验性）v1.0-beta.5");
#endif
    diDeviceList.clear();
    ui->comboBox->clear();

    this->initDirectInput(); // 初始化DirectInput
    this->scanDevice();      // 重新扫描
    // 设备不为空
    if (!diDeviceList.empty()) {
        ui->comboBox->setPlaceholderText("");
        for (auto item : diDeviceList) {
            ui->comboBox->addItem(item.name.data());
        }
    }
    on_comboBox_activated(0); // 默认选择第一个设备

    connect(this, &QWizard::currentIdChanged, this, [=](int id) {
        if (id == 2) {
            updateUserProfile();                                      // 更新用户配置文件
            QStringList gameJoyPosNameList = getDeviceNameGameList(); // 获取游戏配置文件中的设备名称列表
            // 更新到下拉框
            ui->comboBox_2->clear();
            for (auto item : gameJoyPosNameList) {
                ui->comboBox_2->addItem(item);
            }
            if (gameJoyPosNameList.contains(gameDeviceName.data())) {
                ui->comboBox_2->setCurrentText(gameDeviceName.data());
            }
            diDeviceList.clear();
            ui->comboBox->clear();
            this->scanDevice(); // 重新扫描

            // 游戏控制设备不为空
            if (!diDeviceList.empty()) {
                ui->comboBox->setPlaceholderText("");
                for (auto item : diDeviceList) {
                    ui->comboBox->addItem(item.name.data());
                }

                if (hasLastDevInCurrentDeviceList(deviceName)) {
                    ui->comboBox->setCurrentText(deviceName.data());
                    on_comboBox_activated(ui->comboBox->currentIndex());
                } else {
                    ui->comboBox->setCurrentIndex(-1);
                }
            }
        } else if (id == 3) {
            // 连接设备
            if (deviceName.empty()) {
                qDebug() << "设备名称为空！";
                return;
            }
            if (!this->initDirectInput()) {
                qDebug() << "初始化DirectInput失败！";
                return;
            }
            if (!openDiDevice(ui->comboBox->currentIndex(), reinterpret_cast<HWND>(this->winId()))) {
                qDebug() << "打开设备失败！";
                return;
            }

            if (showKeyState == nullptr) {
                showKeyState = new ShowKeyState();
                showKeyState->setWindowTitle("按键状态");
                // 设置坐标为主窗口的左边
                showKeyState->setGeometry(this->geometry().x() - 180, this->geometry().y(), 160, 200);
                showKeyState->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog | Qt::WindowCloseButtonHint); // 设置窗口为置顶
                showKeyState->setKeyCount(capabilities.dwButtons);                                               // 设置按键数量
                showKeyState->setAttribute(Qt::WA_DeleteOnClose);                                                // 关闭时删除窗口对象
                showKeyState->show();                                                                            // 显示按键状态窗口

                connect(showKeyState, &ShowKeyState::destroyed, this, [=]() {
                    if (timer) {
                        timer->stop(); // 停止定时器
                        delete timer;  // 删除定时器对象
                        timer = nullptr;
                    }
                    showKeyState = nullptr; // 释放指针
                });

                // 设置主窗口关闭时，按键状态窗口也关闭
                connect(this, &QWizard::finished, this, [=]() {
                    if (showKeyState) {
                        showKeyState->close(); // 关闭按键状态窗口
                    }
                });

            } else {
                showKeyState->setKeyCount(capabilities.dwButtons); // 设置按键数量
                showKeyState->show();                              // 显示按键状态窗口
            }

            if (timer == nullptr) {
                timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, [=]() {
                    if (pDevice && showKeyState) {
                        BigKey keyState = getKeyState();     // 获取按键状态
                        showKeyState->setKeyState(keyState); // 更新按键状态窗口
                    }
                });
                timer->start(100); // 每100毫秒更新一次
            }
        }
    });
}

ETS2KeyBinderWizard::~ETS2KeyBinderWizard() {
    if (pDevice) {
        pDevice->Unacquire();
        pDevice->Release();
        pDevice = NULL;
    }
    if (pDirectInput) {
        pDirectInput->Release();
        pDirectInput = NULL;
    }
    delete ui;
}

QStringList ETS2KeyBinderWizard::getDeviceNameGameList() {
    QString globalControlsFilePath;
    if (ui->comboBox_4->currentIndex() == 0) {
        globalControlsFilePath = QDir::homePath() + "/Documents/Euro Truck Simulator 2/global_controls.sii";
    } else {
        globalControlsFilePath = QDir::homePath() + "/Documents/American Truck Simulator/global_controls.sii";
    }
    QFile file(globalControlsFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << file.fileName();
        return QStringList();
    }
    QTextStream inGlobal(&file);
    map<QString, QString> deviceNameList;

    QRegularExpression regex1(R"(`di8\.'{(.*?)}\|{(.*?)}'`.*?\|(.+)\s*")");
    while (!inGlobal.atEnd()) {
        QString line = inGlobal.readLine();
        if (line.contains("di8") && line.count("|") == 2) {
            // 截取 GUID 和设备名称
            QRegularExpressionMatch match = regex1.match(line);
            if (match.hasMatch()) {
                QString guid = "{" + match.captured(1) + "}|{" + match.captured(2) + "}";
                QString name = match.captured(3);
                deviceNameList.insert_or_assign(guid, name);
            }
        }
    }
    file.close();

    // 读取配置文件列表
    QFile profileFile(selectedProfilePath);
    if (!profileFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << profileFile.fileName();
        return QStringList();
    }

    QTextStream inProfile(&profileFile);
    QStringList profileList = gameJoyPosNameList;
    QRegularExpression regex2(R"(`di8\.'{(.*?)}\|{(.*?)}'`)");
    while (!inProfile.atEnd()) {
        QString line = inProfile.readLine();
        for (int i = 0; i < gameJoyPosNameList.size(); i++) {
            if (line.contains(gameJoyPosNameList[i])) {
                QRegularExpressionMatch match = regex2.match(line);
                if (match.hasMatch()) {
                    QString guid = "{" + match.captured(1) + "}|{" + match.captured(2) + "}";
                    QString name = deviceNameList[guid];
                    profileList[i] = name + "(" + match.captured(2).left(8) + ")";
                }
            }
        }
    }
    profileFile.close();
    return profileList;
}

bool ETS2KeyBinderWizard::hasLastDevInCurrentDeviceList(std::string lastDeviceName) {
    for (auto item : diDeviceList) {
        if (item.name == lastDeviceName) {
            return true;
        }
    }
    return false;
}

// 备份配置文件
bool ETS2KeyBinderWizard::backupProfile() {
    QFile selectedProfileFile(selectedProfilePath);
    if (!selectedProfileFile.exists()) {
        qDebug() << "配置文件不存在:" << selectedProfilePath;
        return false;
    }

    return QFile::copy(selectedProfilePath, selectedProfilePath + "." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".bak");
}

void ETS2KeyBinderWizard::on_comboBox_activated(int index) {
    deviceName = ui->comboBox->currentText().toStdString();
    if (deviceName.empty()) {
        return;
    }

    // 是否关联设备名称
    if (ui->checkBox->isChecked()) {
        // 检查comboBox_2是否有相同的设备名称，如果有则设置为当前选中项
        for (int i = 0; i < ui->comboBox_2->count(); ++i) {
            if (ui->comboBox_2->itemText(i).trimmed().contains(ui->comboBox->currentText().trimmed())) {
                ui->comboBox_2->setCurrentIndex(i);
                break;
            }
        }
    }
    gameDeviceName = ui->comboBox_2->currentText().toStdString();
}

// 修改 controls.sii 文件
void modifyControlsSii(const QString& controlsFilePath, BindingType bindingType, const QString& ets2BtnStr) {
    QFile controlsFile(controlsFilePath);

    // 检查文件是否存在
    if (!QFileInfo::exists(controlsFilePath)) {
        qWarning() << "文件未找到:" << controlsFilePath;
        return;
    }

    // 替换规则字典
    map<BindingType, QString> replaceRules = {
        {BindingType::lightoff, R"(mix lightoff `.*?semantical\.lightoff\?0`)"},
        {BindingType::lighthorn, R"(mix lighthorn `.*?semantical\.lighthorn\?0`)"},
        {BindingType::wipers0, R"(mix wipers0 `.*?semantical\.wipers0\?0`)"},
        {BindingType::wipers1, R"(mix wipers1 `.*?semantical\.wipers1\?0`)"},
        {BindingType::wipers2, R"(mix wipers2 `.*?semantical\.wipers2\?0`)"},
        {BindingType::wipers3, R"(mix wipers3 `.*?semantical\.wipers3\?0`)"},
        {BindingType::lightpark, R"(mix lightpark `.*?semantical\.lightpark\?0`)"},
        {BindingType::lighton, R"(mix lighton `.*?semantical\.lighton\?0`)"},
        {BindingType::hblight, R"(mix hblight `.*?semantical\.hblight\?0`)"},
        {BindingType::lblinkerh, R"(mix lblinkerh `.*?semantical\.lblinkerh\?0`)"},
        {BindingType::rblinkerh, R"(mix rblinkerh `.*?semantical\.rblinkerh\?0`)"},
        {BindingType::gearsel1off, R"(mix gearsel1off `.*?semantical\.gearsel1off\?0`)"},
        {BindingType::gearsel1on, R"(mix gearsel1on `.*?semantical\.gearsel1on\?0`)"},
        {BindingType::gearsel2off, R"(mix gearsel2off `.*?semantical\.gearsel2off\?0`)"},
        {BindingType::gearsel2on, R"(mix gearsel2on `.*?semantical\.gearsel2on\?0`)"},
    };

    map<BindingType, QString> bindingTypeString = {
        {BindingType::lightoff, "lightoff"},     {BindingType::lighthorn, "lighthorn"},     {BindingType::wipers0, "wipers0"},
        {BindingType::wipers1, "wipers1"},       {BindingType::wipers2, "wipers2"},         {BindingType::wipers3, "wipers3"},
        {BindingType::lightpark, "lightpark"},   {BindingType::lighton, "lighton"},         {BindingType::hblight, "hblight"},
        {BindingType::lblinkerh, "lblinkerh"},   {BindingType::rblinkerh, "rblinkerh"},     {BindingType::gearsel1off, "gearsel1off"},
        {BindingType::gearsel1on, "gearsel1on"}, {BindingType::gearsel2off, "gearsel2off"}, {BindingType::gearsel2on, "gearsel2on"},
    };

    if (replaceRules.find(bindingType) == replaceRules.end()) {
        qWarning() << "无效的绑定类型";
        return;
    }

    // 获取替换规则
    QString pattern = replaceRules[bindingType];
    QString replacement = QString("mix %1 `%2 | semantical.%1?0`").arg(bindingTypeString[bindingType], ets2BtnStr);

    // 打开文件并读取内容
    if (!controlsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << controlsFilePath;
        return;
    }

    QTextStream in(&controlsFile);
    QStringList lines;
    bool modified = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpression regex(pattern);
        if (line.contains(regex)) {
            qDebug() << "匹配到:" << line.trimmed();
            line.replace(regex, replacement);
            modified = true;
        }
        lines.append(line);
    }
    controlsFile.close();

    // 如果有修改，写回文件
    if (modified) {
        if (!controlsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "无法写入文件:" << controlsFilePath;
            return;
        }

        QTextStream out(&controlsFile);
        for (const QString& line : lines) {
            out << line << "\n";
        }
        controlsFile.close();
        qDebug() << "成功修改文件:" << controlsFilePath;
    } else {
        qDebug() << "未检测到需要修改的内容";
    }
}

// 将字符串转换为 ETS2 格式
QString convertToETS2_String(const QString& gameJoyPosStr, const ActionEffect& actionEffect) {
    QString ets2BtnStr;
    if (gameJoyPosStr.isEmpty() || actionEffect.empty()) {
        return ets2BtnStr;
    }
    // 格式：joy3.b10?0 & !joy3.b11?0
    for (auto item : actionEffect) {
        if (item.second == false) {
            ets2BtnStr += "!";
        }
        ets2BtnStr += gameJoyPosStr.trimmed() + ".b" + QString::number(item.first + 1) + "?0 & ";
    }
    ets2BtnStr.chop(3); // 去掉最后的 &
    qDebug() << "转换后的 ETS2 按键字符串:" << ets2BtnStr;
    return ets2BtnStr;
}

// 列出目录下的所有配置文件及其最后修改日期
QList<QPair<QString, QDateTime>> listFoldersWithModificationDates(const QDir& directory) {
    QList<QPair<QString, QDateTime>> folderInfo;

    if (!directory.exists()) {
        qWarning() << "路径" << directory.absolutePath() << "不存在!";
        return folderInfo;
    }

    // 获取目录下的所有子目录
    QFileInfoList folders = directory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& folder : folders) {
        QDateTime lastModified = folder.lastModified();
        folderInfo.append(qMakePair(folder.fileName(), lastModified));
    }

    return folderInfo;
}

// 选择用户配置文件
void ETS2KeyBinderWizard::updateUserProfile() {
    // 列出 steam_profiles 路径下的配置文件
    steamProfileFolders = listFoldersWithModificationDates(steamProfiles[ui->comboBox_4->currentIndex()]);

    // 列出 profiles 路径下的配置文件
    profileFolders = listFoldersWithModificationDates(profiles[ui->comboBox_4->currentIndex()]);

    // 合并两个配置文件列表
    QList<QPair<QString, QDateTime>> allProfileFolders = steamProfileFolders + profileFolders;

    ui->comboBox_3->clear();
    if (allProfileFolders.isEmpty()) {
        qDebug() << "没有找到任何配置文件！";
        return;
    }

    // 遍历所有配置文件，添加到下拉框中
    for (const auto& folder : allProfileFolders) {
        ui->comboBox_3->addItem(folder.first + " (" + folder.second.toString("yyyy-MM-dd HH:mm:ss") + ")");
    }
    on_comboBox_3_activated(0); // 默认选择第一个配置文件
}

// 初始化 DirectInput
bool ETS2KeyBinderWizard::initDirectInput() {
    if (pDirectInput != nullptr) {
        return true;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDirectInput, NULL))) {
        qDebug() << "DirectInput 初始化失败！";
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 初始化失败！");
        return false;
    }

    return true;
}

// 打开选择的设备
bool ETS2KeyBinderWizard::openDiDevice(int deviceIndex, HWND hWnd) {
    if (deviceIndex < 0 || deviceIndex >= diDeviceList.size()) {
        return false;
    }

    // 相同设备, 无需重复打开
    if (pDevice && lastDeviceIndex == deviceIndex) {
        return true;
    }

    // 新设备
    lastDeviceIndex = deviceIndex;
    if (pDevice) {
        pDevice->Unacquire();
        pDevice->Release();
        pDevice = nullptr;
    }

    // 创建设备实例
    if (FAILED(pDirectInput->CreateDevice(diDeviceList[deviceIndex].guidInstance, &pDevice, NULL))) {
        qDebug() << "设备创建失败！";
        QMessageBox::critical(this, "错误", "初始化设备: 设备创建失败！");
        return false;
    }

    if (FAILED(pDevice->SetDataFormat(&c_dfDIJoystick2))) {
        qDebug() << "设置数据格式失败！";
        QMessageBox::critical(this, "错误", "初始化设备: 设置数据格式失败！");
        return false;
    }

    if (FAILED(pDevice->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND))) {
        qDebug() << "设置独占模式失败！";
        QMessageBox::critical(this, "错误", "初始化设备: 设置独占模式失败！");
        return false;
    }

    // 获取控制器能力
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(pDevice->GetCapabilities(&capabilities))) {
        qDebug() << "获取设备能力失败！";
        QMessageBox::critical(this, "错误", "初始化设备: 获取设备能力失败！");
        return false;
    }
    // 获取按钮数量
    qDebug() << "按钮数量:" << capabilities.dwButtons;

    return true;
}

void ETS2KeyBinderWizard::scanDevice() {
    if (pDirectInput == nullptr) {
        return;
    }

    if (FAILED(pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {
        QMessageBox::critical(nullptr, "错误", "扫描设备列表失败！");
        return;
    }
}

// 生成映射文件
bool ETS2KeyBinderWizard::generateMappingFile(ActionEffect hblight, ActionEffect lighthorn) {
    // LightBinder.di_mappings_config 文件格式
    // [
    //     {"dev_btn_name":"按键4", "dev_btn_type":"wheel_button", "dev_btn_value":"0", "keyboard_name":"K", "keyboard_value":37,
    //     "remark":"远光灯", "rotateAxis":0, "btnTriggerType":5, "deviceName":"Generic   USB  Joystick  (00060079)"},
    //     {"dev_btn_name":"按键4+按键7", "dev_btn_type":"wheel_button", "dev_btn_value":"0", "keyboard_name":"J", "keyboard_value":36,
    //     "remark":"灯光喇叭", "rotateAxis":0, "btnTriggerType":0, "deviceName":"Generic   USB  Joystick  (00060079)"}
    // ]

    QString joyName = deviceName.data();
    QFile lightBindingFile(QDir::homePath() + "/AppData/Local/KeyMappingToolData/userMappings/" + MAPPING_FILE_NAME + ".di_mappings_config");
    if (!lightBindingFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << lightBindingFile.fileName();
        return false;
    }
    QString hblightKeyStr, lighthornKeyStr;
    for (auto item : hblight) {
        if (item.second) {
            hblightKeyStr += "按键" + QString::number(item.first) + "+";
        }
    }
    hblightKeyStr.chop(1); // 去掉最后一个 +
    for (auto item : lighthorn) {
        if (item.second) {
            lighthornKeyStr += "按键" + QString::number(item.first) + "+";
        }
    }
    lighthornKeyStr.chop(1); // 去掉最后一个 +

    QTextStream in(&lightBindingFile);
    in << "[\n{\"dev_btn_name\":\"" << hblightKeyStr;
    in << "\", \"dev_btn_type\":\"wheel_button\", \"dev_btn_value\":\"0\", \"keyboard_name\":\"K\", \"keyboard_value\":37, "
          "\"remark\":\"远光灯\", \"rotateAxis\":0, \"btnTriggerType\":5, \"deviceName\":\"";
    in << joyName << "\"},\n";

    in << "{\"dev_btn_name\":\"" << lighthornKeyStr;
    in << "\", \"dev_btn_type\":\"wheel_button\", \"dev_btn_value\":\"0\", \"keyboard_name\":\"J\", \"keyboard_value\":36,"
          "\"remark\":\"灯光喇叭\", \"rotateAxis\":0, \"btnTriggerType\":0, \"deviceName\":\"";
    in << joyName << "\"}\n]";
    return true;
}

bool ETS2KeyBinderWizard::checkHardwareDeviceAndMsgBox() {
    if (deviceName.empty()) {
        QMessageBox box(QMessageBox::Critical, "错误", "请将您的设备连接到电脑！\n" + MEG_BOX_LINE + "\n或者进行手动绑定？");
        box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Ok);
        box.exec();
        if (box.clickedButton() == box.button(QMessageBox::Ok)) {
            on_pushButton_16_clicked();
        }
        return false;
    }
    return true;
}

void ETS2KeyBinderWizard::oneKeyBind(BindingType bindingType, const QString& message) {
    if (checkHardwareDeviceAndMsgBox() == false) {
        return; // 设备未连接，取消操作
    }

    BigKey keyState[1];          // 记录按键状态
    keyState[0] = getKeyState(); // 获取按键状态，第一次获取为0，应该是BUG

    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "单操作绑定", message);
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Ok);
    box.exec();
    if (box.clickedButton() == box.button(QMessageBox::Cancel)) {
        return; // 取消操作
    }
    keyState[0] = getKeyState(); // 获取按键状态
    size_t keyPressCount = 0;
    size_t keyPos = 0;
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyState[0].getBit(i)) {
            keyPressCount++;
            keyPos = i;
        }
    }
    if (keyPressCount < 1) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }
    if (keyPressCount > 1) {
        QMessageBox::critical(this, "错误", "找到多个按键按下！请重新操作！");
        return;
    }
    // 2、确定是否绑定
    box.setText("是否绑定？");
    int ret = box.exec();
    if (ret == QMessageBox::Ok) {
        backupProfile(); // 备份配置文件
        int gameJoyPosIndex = ui->comboBox_2->currentIndex();
        QString ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex].trimmed() + ".b" + QString::number(keyPos + 1) + "?0";
        modifyControlsSii(selectedProfilePath, bindingType, ets2BtnStr);
    }
}

void ETS2KeyBinderWizard::multiKeyBind(std::map<BindingType, ActionEffect> actionEffectMap) {
    QMessageBox box(QMessageBox::Information, "多操作绑定", "是否绑定？");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = box.exec();
    if (ret == QMessageBox::Ok) {
        backupProfile(); // 备份配置文件
        QString gameJoyPosStr = gameJoyPosNameList[ui->comboBox_2->currentIndex()];
        for (auto item : actionEffectMap) {
            QString ets2BtnStr = convertToETS2_String(gameJoyPosStr, item.second);
            modifyControlsSii(selectedProfilePath, item.first, ets2BtnStr);
        }
    }
}

std::vector<BigKey> ETS2KeyBinderWizard::getMultiKeyState(const QString& title, const QStringList& messages) {
    if (checkHardwareDeviceAndMsgBox() == false) {
        return {}; // 设备未连接，取消操作
    }

    int stepSum = messages.size(); // 步骤总数
    if (stepSum < 1) {
        return {};
    }
    std::vector<BigKey> keyStates(stepSum); // 记录按键状态
    keyStates[0] = getKeyState();           // 获取按键状态，第一次获取为0，应该是BUG

    for (int i = 0; i < stepSum; ++i) {
        QMessageBox box(QMessageBox::Information, title + ":" + QString::number(i + 1) + "/" + QString::number(stepSum), messages[i]);
        box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Ok);
        box.exec();
        if (box.clickedButton() == box.button(QMessageBox::Cancel)) {
            return {}; // 取消操作
        }
        keyStates[i] = getKeyState(); // 获取按键状态
    }
    return keyStates;
}

// 1、示廓灯&近光灯
void ETS2KeyBinderWizard::on_pushButton_clicked() {
    QString messageTitle = "示廓灯&近光灯";
    QStringList messages = {
        "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n关闭灯光",
        "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n示廓灯",
        "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n近光灯",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    std::map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::lightoff, ActionEffect()},
        {BindingType::lightpark, ActionEffect()},
        {BindingType::lighton, ActionEffect()},
    };
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[2].getBit(i) != keyStates[0].getBit(i) || keyStates[2].getBit(i) != keyStates[1].getBit(i)) {
            actionEffectMap[BindingType::lightoff].insert_or_assign(i, keyStates[0].getBit(i));
            actionEffectMap[BindingType::lightpark].insert_or_assign(i, keyStates[1].getBit(i));
            actionEffectMap[BindingType::lighton].insert_or_assign(i, keyStates[2].getBit(i));
        }
    }

    multiKeyBind(actionEffectMap); // 多按键绑定
}

// 2、远光灯&灯光喇叭
void ETS2KeyBinderWizard::on_pushButton_2_clicked() {
    QString messageTitle = "远光灯&灯光喇叭";
    QStringList messages = {
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭灯光",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n远光灯",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n灯光喇叭",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    // 找出变化按键
    std::map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::hblight, ActionEffect()},
        {BindingType::lighthorn, ActionEffect()},
    };
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[1].getBit(i) != keyStates[0].getBit(i) || keyStates[2].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::hblight].insert_or_assign(i, keyStates[1].getBit(i));
            actionEffectMap[BindingType::lighthorn].insert_or_assign(i, keyStates[2].getBit(i));
        }
    }
    if (actionEffectMap[BindingType::hblight].size() < 1 || actionEffectMap[BindingType::lighthorn].size() < 1) {
        QMessageBox::critical(this, "错误", "部分操作没有找到变化的按键！");
        return;
    }
    if (keyStates[1] == keyStates[2]) {
        QMessageBox box(QMessageBox::Information, "提示", "远光灯和灯光喇叭的按键相同\n" + MEG_BOX_LINE + "\n您是想绑定：",
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        box.setDefaultButton(QMessageBox::Cancel);
        box.setButtonText(QMessageBox::Yes, "远光灯");
        box.setButtonText(QMessageBox::No, "灯光喇叭");
        box.exec();
        if (box.clickedButton() == box.button(QMessageBox::Cancel)) {
            return; // 取消操作
        } else if (box.clickedButton() == box.button(QMessageBox::Yes)) {
            multiKeyBind(std::map<BindingType, ActionEffect>{{BindingType::hblight, actionEffectMap[BindingType::hblight]}});
        } else if (box.clickedButton() == box.button(QMessageBox::No)) {
            multiKeyBind(std::map<BindingType, ActionEffect>{{BindingType::lighthorn, actionEffectMap[BindingType::lighthorn]}});
        }
    } else {

        // 4、生成配置文件
        generateMappingFile(actionEffectMap[BindingType::hblight], actionEffectMap[BindingType::lighthorn]);
        backupProfile(); // 备份配置文件
        modifyControlsSii(selectedProfilePath, BindingType::hblight, "keyboard.k?0");
        modifyControlsSii(selectedProfilePath, BindingType::lighthorn, "keyboard.j?0");

        QMessageBox box(QMessageBox::Information, "远光灯&灯光喇叭", "");
#if defined(INDEPENDENT_MODE)
        box.setText("游戏不支持开关类型的远光灯绑定，已生成配置文件 \"" + MAPPING_FILE_NAME + "\"\n"
                    + "请打开“KeyMappingsTool”使用此配置文件，间接实现拨杆映射。\n\n配置文件路径：\n" + QDir::homePath()
                    + "/AppData/Local/KeyMappingToolData/userMappings/" + MAPPING_FILE_NAME + ".di_mappings_config\n\n"
                    + "KeyMappingsTool 下载地址：\nhttps://github.com/InsistonTan/KeyMappingsTool/releases"
                      "\n\n是否尝试打开“KeyMappingsTool”？");
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.exec();
        if (box.clickedButton() == box.button(QMessageBox::Yes)) {
            // 打开 KeyMappingsTool，当自身程序关闭时，KeyMappingsTool不需要关闭
            QString keyMappingsToolPath = "KeyMappingsTool.exe";
            QProcess::startDetached(keyMappingsToolPath);
        }
#else
        // 此组件合并至 KeyMappingsTool 中，不再单独使用，所以不需要提示用户打开 KeyMappingsTool
        box.setText("游戏不支持开关类型的远光灯绑定\n\n"
                    "请回到主界面\n"
                    "选择设备：“"
                    + QString::fromStdString(deviceName)
                    + "”\n"
                      "选择映射：“映射键盘”\n"
                      "配置文件：\""
                    + MAPPING_FILE_NAME + "\"");
        box.setStandardButtons(QMessageBox::Yes);
        box.exec();
#endif
    }
}

// 3、左转向灯&右转向灯
void ETS2KeyBinderWizard::on_pushButton_3_clicked() {
    QString messageTitle = "左转向灯&右转向灯";
    QStringList messages = {
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭灯光",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n左转向灯",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n右转向灯",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::lblinkerh, ActionEffect()},
        {BindingType::rblinkerh, ActionEffect()},
    };
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[1].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::lblinkerh].insert_or_assign(i, keyStates[1].getBit(i));
        }
        if (keyStates[2].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::rblinkerh].insert_or_assign(i, keyStates[2].getBit(i));
        }
    }
    if (actionEffectMap[BindingType::lblinkerh].size() < 1 || actionEffectMap[BindingType::rblinkerh].size() < 1) {
        QMessageBox::critical(this, "错误", "部分操作没有找到变化的按键！");
        return;
    }
    multiKeyBind(actionEffectMap); // 多按键绑定
}

// 4、雨刮器
void ETS2KeyBinderWizard::on_pushButton_4_clicked() {
    QString messageTitle = "雨刮器";
    QStringList messages = {
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刮器1档",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刮器2档",
        "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刮器3档",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    std::map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::wipers0, ActionEffect()},
        {BindingType::wipers1, ActionEffect()},
        {BindingType::wipers2, ActionEffect()},
        {BindingType::wipers3, ActionEffect()},
    };

    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[1].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::wipers0].insert_or_assign(i, keyStates[0].getBit(i));
            actionEffectMap[BindingType::wipers1].insert_or_assign(i, keyStates[1].getBit(i));
        }
        if (keyStates[2].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::wipers0].insert_or_assign(i, keyStates[0].getBit(i));
            actionEffectMap[BindingType::wipers2].insert_or_assign(i, keyStates[2].getBit(i));
        }
        if (keyStates[3].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::wipers0].insert_or_assign(i, keyStates[0].getBit(i));
            actionEffectMap[BindingType::wipers3].insert_or_assign(i, keyStates[3].getBit(i));
        }
    }
    if (actionEffectMap[BindingType::wipers0].size() < 1 || actionEffectMap[BindingType::wipers1].size() < 1
        || actionEffectMap[BindingType::wipers2].size() < 1) {
        QMessageBox::critical(this, "错误", "部分操作没有找到变化的按键！");
        return;
    }
    multiKeyBind(actionEffectMap); // 多按键绑定
}

// 5、档位开关1
void ETS2KeyBinderWizard::on_pushButton_18_clicked() {
    QString messageTitle = "档位开关1";
    QStringList messages = {
        "档位开关1拨到：\n" + MEG_BOX_LINE + "\n低档位",
        "档位开关1拨到：\n" + MEG_BOX_LINE + "\n高档位",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::gearsel1off, ActionEffect()},
        {BindingType::gearsel1on, ActionEffect()},
    };
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[1].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::gearsel1on].insert_or_assign(i, keyStates[1].getBit(i));
            actionEffectMap[BindingType::gearsel1off].insert_or_assign(i, keyStates[0].getBit(i));
        }
    }
    if (actionEffectMap[BindingType::gearsel1off].size() < 1) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }
    multiKeyBind(actionEffectMap); // 多按键绑定
}

// 6、档位开关2
void ETS2KeyBinderWizard::on_pushButton_19_clicked() {
    QString messageTitle = "档位开关2";
    QStringList messages = {
        "档位开关2拨到：\n" + MEG_BOX_LINE + "\n低档位",
        "档位开关2拨到：\n" + MEG_BOX_LINE + "\n高档位",
    };
    std::vector<BigKey> keyStates = getMultiKeyState(messageTitle, messages);
    if (keyStates.empty()) {
        return; // 取消操作
    }

    map<BindingType, ActionEffect> actionEffectMap = {
        {BindingType::gearsel2off, ActionEffect()},
        {BindingType::gearsel2on, ActionEffect()},
    };
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (keyStates[1].getBit(i) != keyStates[0].getBit(i)) {
            actionEffectMap[BindingType::gearsel2on].insert_or_assign(i, keyStates[1].getBit(i));
            actionEffectMap[BindingType::gearsel2off].insert_or_assign(i, keyStates[0].getBit(i));
        }
    }
    if (actionEffectMap[BindingType::gearsel2off].size() < 1) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }
    multiKeyBind(actionEffectMap); // 多按键绑定
}

BigKey ETS2KeyBinderWizard::getKeyState() {
    if (pDevice == nullptr) {
        return BigKey(); // 设备未打开
    }

    BigKey keyState;
    DIJOYSTATE2 js;
    pDevice->Acquire();
    HRESULT hr = pDevice->Poll();

    // 检查连接状态
    if (FAILED(hr)) {
        hr = pDevice->Acquire();
    }

    // 检查是否成功获取
    if (FAILED(hr)) {
        qDebug() << "设备poll()失败，错误代码：" << HRESULT_CODE(hr);
        return keyState;
    }

    if (SUCCEEDED(pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
        // 获取按键状态
        for (size_t i = 0; i < capabilities.dwButtons; i++) {
            keyState.setBit(i, (js.rgbButtons[i] & 0x80));
        }
    } else {
        qDebug() << "获取设备状态信息失败!";
        qDebug() << "GetDeviceState failed with error:" << HRESULT_CODE(hr);
    }

    return keyState;
}

void ETS2KeyBinderWizard::on_comboBox_3_activated(int index) {
    if (index >= 0 && index < steamProfileFolders.size()) {
        selectedProfilePath = steamProfiles[ui->comboBox_4->currentIndex()] + "/" + steamProfileFolders[index].first + "/controls.sii";
    } else if (index >= steamProfileFolders.size() && index < steamProfileFolders.size() + profileFolders.size()) {
        selectedProfilePath =
            profiles[ui->comboBox_4->currentIndex()] + "/" + profileFolders[index - steamProfileFolders.size()].first + "/controls.sii";
    } else {
        qDebug() << "无效的配置文件索引:" << index;
    }
}

void ETS2KeyBinderWizard::on_comboBox_2_activated(int index) {
    gameDeviceName = ui->comboBox_2->currentText().toStdString();
}

void ETS2KeyBinderWizard::on_checkBox_3_clicked(bool checked) {
    ui->stackedWidget->setCurrentIndex(checked);
}

// 手动绑定
void ETS2KeyBinderWizard::on_pushButton_16_clicked() {
    ManuallyBinder* manuallyBinder = new ManuallyBinder(this);
    manuallyBinder->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
    // 当manuallyBinder没关闭时，不允许操作其他窗口
    manuallyBinder->setWindowModality(Qt::ApplicationModal); // 设置窗口模式为应用程序模态

    if (pDirectInput != nullptr) {
        manuallyBinder->setKeyCount(capabilities.dwButtons); // 设置按键数量
    } else {
        manuallyBinder->setKeyCount(128); // 设置按键数量
    }

    // 连接信号槽
    connect(manuallyBinder, &ManuallyBinder::keyBound, this, &ETS2KeyBinderWizard::modifyControlsSii_Slot, Qt::DirectConnection);
    manuallyBinder->show();
}

void ETS2KeyBinderWizard::modifyControlsSii_Slot(BindingType bindingType, ActionEffect actionEffect) {
    if (actionEffect.empty()) {
        qDebug() << "没有受影响的按键！";
        return;
    }

    QString ets2BtnStr;
    for (const auto& action : actionEffect) {
        int keyIndex = action.first; // 获取按键索引
        if (action.second == false) {
            ets2BtnStr += "!";
        }
        ets2BtnStr += gameJoyPosNameList[ui->comboBox_2->currentIndex()].trimmed() + ".b" + QString::number(keyIndex + 1) + " & "; // 1基索引
    }
    ets2BtnStr.chop(3); // 去掉最后的 " & "

    qDebug() << "修改的按键:" << ets2BtnStr;
    backupProfile(); // 备份配置文件
    modifyControlsSii(selectedProfilePath, bindingType, ets2BtnStr);
}

// 关闭灯光
void ETS2KeyBinderWizard::on_pushButton_5_clicked() {
    oneKeyBind(BindingType::lightoff, "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n关闭灯光");
}

// 示廊灯
void ETS2KeyBinderWizard::on_pushButton_6_clicked() {
    oneKeyBind(BindingType::lightpark, "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n示廊灯");
}

// 近光灯
void ETS2KeyBinderWizard::on_pushButton_7_clicked() {
    oneKeyBind(BindingType::lighton, "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n近光灯");
}

// 左转向灯
void ETS2KeyBinderWizard::on_pushButton_8_clicked() {
    oneKeyBind(BindingType::lblinkerh, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n左转向灯");
}

// 右转向灯
void ETS2KeyBinderWizard::on_pushButton_9_clicked() {
    oneKeyBind(BindingType::rblinkerh, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n右转向灯");
}

// 关闭雨刷
void ETS2KeyBinderWizard::on_pushButton_10_clicked() {
    oneKeyBind(BindingType::wipers0, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭雨刷");
}

// 雨刷1档
void ETS2KeyBinderWizard::on_pushButton_11_clicked() {
    oneKeyBind(BindingType::wipers1, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刷1档");
}

// 雨刷2档
void ETS2KeyBinderWizard::on_pushButton_12_clicked() {
    oneKeyBind(BindingType::wipers2, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刷2档");
}

// 雨刷3档
void ETS2KeyBinderWizard::on_pushButton_13_clicked() {
    oneKeyBind(BindingType::wipers3, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n雨刷3档");
}

// 远光灯
void ETS2KeyBinderWizard::on_pushButton_14_clicked() {
    oneKeyBind(BindingType::hblight, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n远光灯");
}

// 灯光喇叭
void ETS2KeyBinderWizard::on_pushButton_15_clicked() {
    oneKeyBind(BindingType::lighthorn, "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n灯光喇叭");
}

// 12、档位开关1关闭
void ETS2KeyBinderWizard::on_pushButton_20_clicked() {
    oneKeyBind(BindingType::gearsel1off, "请将档位开关1拨到：\n" + MEG_BOX_LINE + "\n低档位");
}

// 13、档位开关1打开
void ETS2KeyBinderWizard::on_pushButton_21_clicked() {
    oneKeyBind(BindingType::gearsel1on, "请将档位开关1拨到：\n" + MEG_BOX_LINE + "\n高档位");
}

// 14、档位开关2关闭
void ETS2KeyBinderWizard::on_pushButton_22_clicked() {
    oneKeyBind(BindingType::gearsel2off, "请将档位开关2拨到：\n" + MEG_BOX_LINE + "\n低档位");
}

// 15、档位开关2打开
void ETS2KeyBinderWizard::on_pushButton_23_clicked() {
    oneKeyBind(BindingType::gearsel2on, "请将档位开关2拨到：\n" + MEG_BOX_LINE + "\n高档位");
}

// 接线提示
void ETS2KeyBinderWizard::on_pushButton_17_clicked() {
    QLabel* labelImage = new QLabel(this, Qt::Dialog | Qt::WindowCloseButtonHint);
    labelImage->setWindowTitle("接线提示");

    QString imagePath = ":/ETS2_KeyBinder/ConnectTip_WuLing.jpg";

    QFileInfo file(imagePath);

    if (file.exists()) {
        QImage image;
        image.load(imagePath);

        // Label跟随图片大小变化
        labelImage->resize(QSize(image.width(), image.height()));
        labelImage->setPixmap(QPixmap::fromImage(image));

    } else {
        qDebug() << "未找到该图片";
    }
    labelImage->show();
}
