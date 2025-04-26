#include "ets2keybinderwizard.h"
#include "global.h"
#include "ui_ets2keybinderwizard.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <map>
#include <regex>
#include <string>

// 参考开源项目：https://github.com/Sab1e-GitHub/ETS2-KeyBinder

const QString LOG_HEADER = "<b>特殊按键绑定</b> - ";
const QString MEG_BOX_LINE = "------------------------";
const QString MAPPING_FILE_NAME = "LightBinder";

using namespace std;

// 欧卡2 设置     “摇杆 Button0” 0基索引
// 欧卡2 配置文件 “joy.b1”      1基索引
// pygame Button_Index         0基索引

// 枚举类型定义
enum class BindingType
{
    lightoff,  // 关闭灯光
    lighthorn, // 灯光喇叭
    wipers0,   // 雨刷器关闭
    wipers1,   // 雨刷器1档
    wipers2,   // 雨刷器2档
    wipers3,   // 雨刷器3档
    lightpark, // 示廓灯
    lighton,   // 近光灯
    hblight,   // 远光灯
    lblinkerh, // 左转向灯
    rblinkerh  // 右转向灯
};

QString steamProfiles(QDir::homePath() + "/Documents/Euro Truck Simulator 2/steam_profiles");
QString profiles(QDir::homePath() + "/Documents/Euro Truck Simulator 2/profiles");

QStringList gameJoyPosNameList = {
    "joy", "joy2", "joy3", "joy4", "joy5",
};

ETS2KeyBinderWizard::ETS2KeyBinderWizard(QWidget* parent) : QWizard(parent), ui(new Ui::ETS2KeyBinderWizard) {
    ui->setupUi(this);
    diDeviceList.clear();
    ui->comboBox->clear();

    scanDevice(); // 重新扫描
    // 设备不为空
    if (!diDeviceList.empty()) {
        ui->comboBox->setPlaceholderText("");
        for (auto item : diDeviceList) {
            ui->comboBox->addItem(item.name.data());
        }
    }
    deviceName = ui->comboBox->currentText().toStdString();
    updateUserProfile(); // 更新用户配置文件

    connect(this, &QWizard::currentIdChanged, this, [=](int id) {
        if (id == 2) {
            diDeviceList.clear();
            ui->comboBox->clear();
            scanDevice(); // 重新扫描

            // 游戏控制设备不为空
            if (!diDeviceList.empty()) {
                ui->comboBox->setPlaceholderText("");
                for (auto item : diDeviceList) {
                    ui->comboBox->addItem(item.name.data());
                }

                if (hasLastDevInCurrentDeviceList(deviceName)) {
                    ui->comboBox->setCurrentText(deviceName.data());
                } else {
                    ui->comboBox->setCurrentIndex(-1);
                }
            }
        } else if (id == 3) {
            // 备份配置文件
            backupProfile();
            // 连接设备
            if (deviceName.empty()) {
                return;
            }
            if (!initDirectInput()) {
                return;
            }
            if (!openDiDevice(ui->comboBox->currentIndex(), reinterpret_cast<HWND>(this->winId()))) {
                return;
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

    return QStringList();
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
    int index = ui->comboBox_3->currentIndex();
    if (index >= 0 && index < steamProfileFolders.size()) {
        selectedProfilePath = steamProfiles + "/" + steamProfileFolders[index].first;
    } else if (index >= steamProfileFolders.size() && index < steamProfileFolders.size() + profileFolders.size()) {
        selectedProfilePath = profiles + "/" + profileFolders[index - steamProfileFolders.size()].first;
    } else {
        qDebug() << "无效的配置文件索引:" << index;
        return false;
    }

    QDir dir(selectedProfilePath);
    if (!dir.exists()) {
        qDebug() << "配置文件目录不存在:" << selectedProfilePath;
        return false;
    }

    return QFile::copy(selectedProfilePath + "/controls.sii",
                       selectedProfilePath + "/controls.sii." + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".bak");
}

void ETS2KeyBinderWizard::on_comboBox_activated(int index) {
    deviceName = ui->comboBox->currentText().toStdString();
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
    };

    map<BindingType, QString> bindingTypeString = {
        {BindingType::lightoff, "lightoff"},   {BindingType::lighthorn, "lighthorn"}, {BindingType::wipers0, "wipers0"},
        {BindingType::wipers1, "wipers1"},     {BindingType::wipers2, "wipers2"},     {BindingType::wipers3, "wipers3"},
        {BindingType::lightpark, "lightpark"}, {BindingType::lighton, "lighton"},     {BindingType::hblight, "hblight"},
        {BindingType::lblinkerh, "lblinkerh"}, {BindingType::rblinkerh, "rblinkerh"}};

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
    steamProfileFolders = listFoldersWithModificationDates(steamProfiles);

    // 列出 profiles 路径下的配置文件
    profileFolders = listFoldersWithModificationDates(profiles);

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
    selectedProfilePath = steamProfiles + steamProfileFolders[0].first; // 默认选择第一个配置文件
}

// 初始化 DirectInput
bool ETS2KeyBinderWizard::initDirectInput() {
    if (pDirectInput != nullptr) {
        return true;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDirectInput, NULL))) {
        pushToQueue(parseSuccessLog(LOG_HEADER + "DirectInput 初始化失败！"));
        qDebug() << "DirectInput 初始化失败！";
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 初始化失败！");
        return false;
    }

    pushToQueue(parseSuccessLog(LOG_HEADER + "DirectInput 初始化成功！"));

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

// 生成映射文件
bool ETS2KeyBinderWizard::generateMappingFile(int keyIndex1, int keyIndex2) {
    // LightBinder.di_mappings_config 文件格式
    // [
    //     {"dev_btn_name":"按键4", "dev_btn_type":"wheel_button", "dev_btn_value":"0", "keyboard_name":"K", "keyboard_value":37, "remark":"远光灯",
    //     "rotateAxis":0, "btnTriggerType":5},
    //     {"dev_btn_name":"按键4+按键7", "dev_btn_type":"wheel_button", "dev_btn_value":"0", "keyboard_name":"J", "keyboard_value":36,
    //     "remark":"灯光喇叭", "rotateAxis":0, "btnTriggerType":0}
    // ]
    QFile lightBindingFile(QDir::homePath() + "/AppData/Local/KeyMappingToolData/userMappings/" + MAPPING_FILE_NAME + ".di_mappings_config");
    if (!lightBindingFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << lightBindingFile.fileName();
        return false;
    }
    QTextStream in(&lightBindingFile);
    in << "[\n{\"dev_btn_name\":\"按键" << QString::number(keyIndex1)
       << "\", \"dev_btn_type\":\"wheel_button\", \"dev_btn_value\":\"0\", \"keyboard_name\":\"K\", \"keyboard_value\":37, "
          "\"remark\":\"远光灯\", \"rotateAxis\":0, \"btnTriggerType\":5},\n";
    in << "{\"dev_btn_name\":\"按键" << QString::number(keyIndex1) + "+按键" << QString::number(keyIndex2)
       << "\", \"dev_btn_type\":\"wheel_button\", \"dev_btn_value\":\"0\", \"keyboard_name\":\"J\", \"keyboard_value\":36, "
          "\"remark\":\"灯光喇叭\", \"rotateAxis\":0, \"btnTriggerType\":0}\n]\n";
    return true;
}

// 1、示廓灯&近光灯
void ETS2KeyBinderWizard::on_pushButton_clicked() {
    int stepSum = 3;             // 步骤总数
    BigKey keyState[3];          // 记录按键状态
    keyState[0] = getKeyState(); // 获取按键状态，第一次获取为0，应该是BUG

    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "示廓灯&近光灯：1/" + QString::number(stepSum), "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n关闭灯光");
    box.exec();
    keyState[0] = getKeyState(); // 获取按键状态

    // 2、将拨杆拨到示廓灯位置
    box.setWindowTitle("示廓灯&近光灯：2/" + QString::number(stepSum));
    box.setText("请将拨杆拧到：\n" + MEG_BOX_LINE + "\n示廓灯");
    box.exec();
    keyState[1] = getKeyState(); // 获取按键状态

    // 3、将拨杆拨到近光灯位置
    box.setWindowTitle("示廓灯&近光灯：3/" + QString::number(stepSum));
    box.setText("请将拨杆拧到：\n" + MEG_BOX_LINE + "\n近光灯");
    box.exec();
    keyState[2] = getKeyState(); // 获取按键状态

    // 找出变化按键，实际上不止一个变化
    BigKey diffKey1 = keyState[0] ^ keyState[1]; // 关闭灯光和示廓灯的异或
    BigKey diffKey2 = keyState[0] ^ keyState[2]; // 关闭灯光和近光灯的异或

    std::vector<int> diffKey1Index, diffKey2Index;
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (diffKey1.getBit(i)) {
            diffKey1Index.push_back(i);
        }
        if (diffKey2.getBit(i)) {
            diffKey2Index.push_back(i);
        }
    }
    if (diffKey1Index.size() < 1 || diffKey2Index.size() < 1) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }

    // 4、确定是否绑定
    box.setWindowTitle("示廓灯&近光灯");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);

    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // config_lines[395]: "mix lightoff `!joy3.b10?0 | semantical.lightoff?0`"
        // config_lines[396]: "mix lightpark `joy3.b10?0 & !joy3.b11?0 | semantical.lightpark?0`"
        // config_lines[397]: "mix lighton `joy3.b10?0 & joy3.b11?0 | semantical.lighton?0`"
        int gameJoyPosIndex = ui->comboBox_2->currentIndex();
        QString ets2BtnStr = "!" + gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index[0] + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::lightoff, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index[0] + 1) + "?0";
        if (diffKey2Index.size() > 1) {
            ets2BtnStr += " & !" + gameJoyPosNameList[gameJoyPosIndex] + ".b"
                          + QString::number((diffKey2Index[0] == diffKey1Index[0] ? diffKey2Index[1] : diffKey2Index[0]) + 1) + "?0";
        }
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::lightpark, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index[0] + 1) + "?0";
        if (diffKey2Index.size() > 1) {
            ets2BtnStr += " & " + gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey2Index[1] + 1) + "?0";
        }
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::lighton, ets2BtnStr);
    }
}

// 2、远光灯&灯光喇叭
void ETS2KeyBinderWizard::on_pushButton_2_clicked() {
    int stepSum = 3;             // 步骤总数
    BigKey keyState[3];          // 记录按键状态
    keyState[0] = getKeyState(); // 获取按键状态，第一次获取为0，应该是BUG

    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "远光灯&灯光喇叭：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    keyState[0] = getKeyState(); // 获取按键状态

    // 2、将拨杆拨到远光灯位置
    box.setWindowTitle("远光灯&灯光喇叭：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n远光灯");
    box.exec();
    keyState[1] = getKeyState(); // 获取按键状态

    // 3、将拨杆拨到灯光喇叭位置
    box.setWindowTitle("远光灯&灯光喇叭：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n灯光喇叭");
    box.exec();
    keyState[2] = getKeyState(); // 获取按键状态

    // 找出变化按键，实际上不止一个变化
    BigKey diffKey1 = keyState[0] ^ keyState[1]; // 关闭灯光和远光灯的异或
    BigKey diffKey2 = keyState[0] ^ keyState[2]; // 关闭灯光和灯光喇叭的异或
    BigKey diffKey3 = keyState[1] ^ keyState[2]; // 远光灯和灯光喇叭的异或

    vector<int> diffKey1Index, diffKey2Index, diffKey3Index;
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (diffKey1.getBit(i)) {
            diffKey1Index.push_back(i);
        }
        if (diffKey2.getBit(i)) {
            diffKey2Index.push_back(i);
        }
        if (diffKey3.getBit(i)) {
            diffKey3Index.push_back(i);
        }
    }
    if (diffKey1Index.size() < 1 || diffKey2Index.size() < 1 || diffKey3Index.size() < 1) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }

    // 默认     01
    // 远光灯   10
    // 灯光喇叭 11

    if (diffKey1Index.size() > 1) {
        if (keyState[1].getBit(diffKey1Index[0])) {
            // 远光灯在前，灯光喇叭在后
            generateMappingFile(diffKey1Index[0], diffKey1Index[1]);
        } else {
            generateMappingFile(diffKey1Index[1], diffKey1Index[0]);
        }
    }
    // 4、生成配置文件
    box.setWindowTitle("远光灯&灯光喇叭");
    box.setText("欧卡不支持远光灯的同步绑定，已生成配置文件 <<" + MAPPING_FILE_NAME + ">>，请回到主界面后用此配置文件开启全局映射。");
    box.exec();
}

// 3、左转向灯&右转向灯
void ETS2KeyBinderWizard::on_pushButton_3_clicked() {
    int stepSum = 3;             // 步骤总数
    BigKey keyState[3];          // 记录按键状态
    keyState[0] = getKeyState(); // 获取按键状态，第一次获取为0，应该是BUG

    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "左转向灯&右转向灯：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    keyState[0] = getKeyState();

    // 2、将拨杆拨到左转向灯位置
    box.setWindowTitle("左转向灯&右转向灯：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n左转向灯");
    box.exec();
    keyState[1] = getKeyState(); // 获取按键状态

    // 3、将拨杆拨到右转向灯位置
    box.setWindowTitle("左转向灯&右转向灯：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n右转向灯");
    box.exec();
    keyState[2] = getKeyState(); // 获取按键状态

    // 找出变化按键
    BigKey diffKey1 = keyState[0] ^ keyState[1]; // 关闭灯光和左转向灯的异或
    BigKey diffKey2 = keyState[0] ^ keyState[2]; // 关闭灯光和右转向灯的异或

    int diffKey1Index = -1, diffKey2Index = -1;
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (diffKey1.getBit(i)) {
            diffKey1Index = i;
        }
        if (diffKey2.getBit(i)) {
            diffKey2Index = i;
        }
    }
    if (diffKey1Index < 0 || diffKey2Index < 0) {
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }

    // 4、确定是否绑定
    box.setWindowTitle("左转向灯&右转向灯");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);
    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // config_lines[400]: "mix lblinkerh `joy3.b6?0 | semantical.lblinkerh?0`"
        // config_lines[402]: "mix rblinkerh `joy3.b7?0 | semantical.rblinkerh?0`"
        int gameJoyPosIndex = ui->comboBox_2->currentIndex();
        QString ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::lblinkerh, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey2Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::rblinkerh, ets2BtnStr);
    }
}

// 4、雨刮器
void ETS2KeyBinderWizard::on_pushButton_4_clicked() {
    int stepSum = 4;             // 步骤总数
    BigKey keyState[4];          // 记录按键状态
    keyState[0] = getKeyState(); // 获取按键状态，第一次获取为0，应该是BUG

    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "雨刮器：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    keyState[0] = getKeyState();

    // 2、将拨杆拨到雨刮器1档位置
    box.setWindowTitle("雨刮器：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器1档");
    box.exec();
    keyState[1] = getKeyState(); // 获取按键状态

    // 3、将拨杆拨到雨刮器2档位置
    box.setWindowTitle("雨刮器：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器2档");
    box.exec();
    keyState[2] = getKeyState(); // 获取按键状态

    // 4、将拨杆拨到雨刮器3档位置
    box.setWindowTitle("雨刮器：4/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器3档");
    box.exec();
    keyState[3] = getKeyState(); // 获取按键状态

    // 找出变化按键
    BigKey diffKey1 = keyState[0] ^ keyState[1]; // 关闭灯光和雨刮器1档的异或
    BigKey diffKey2 = keyState[0] ^ keyState[2]; // 关闭灯光和雨刮器2档的异或
    BigKey diffKey3 = keyState[0] ^ keyState[3]; // 关闭灯光和雨刮器3档的异或

    int diffKey1Index = -1, diffKey2Index = -1, diffKey3Index = -1;
    for (size_t i = 0; i < capabilities.dwButtons; i++) {
        if (diffKey1.getBit(i)) {
            diffKey1Index = i;
        }
        if (diffKey2.getBit(i)) {
            diffKey2Index = i;
        }
        if (diffKey3.getBit(i)) {
            diffKey3Index = i;
        }
    }
    if (diffKey1Index < 0 || diffKey2Index < 0 || diffKey3Index < 0) {
        qDebug() << "没有找到变化的按键！";
        QMessageBox::critical(this, "错误", "没有找到变化的按键！");
        return;
    }

    // 5、确定是否绑定
    box.setWindowTitle("雨刮器 绑定");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);
    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // config_lines[383]: "mix wipers0 `!joy3.b1?0 & !joy3.b2?0 & !joy3.b3?0 | semantical.wipers0?0`"
        // config_lines[384]: "mix wipers1 `joy3.b1?0 | semantical.wipers1?0`"
        // config_lines[385]: "mix wipers2 `joy3.b2?0 | semantical.wipers2?0`"
        // config_lines[386]: "mix wipers3 `joy3.b3?0 | semantical.wipers3?0`"
        int gameJoyPosIndex = ui->comboBox_2->currentIndex();
        QString ets2BtnStr = "!" + gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index + 1) + "?0 & !"
                             + gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey2Index + 1) + "?0 & !"
                             + gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey3Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::wipers0, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey1Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::wipers1, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey2Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::wipers2, ets2BtnStr);
        ets2BtnStr = gameJoyPosNameList[gameJoyPosIndex] + ".b" + QString::number(diffKey3Index + 1) + "?0";
        modifyControlsSii(selectedProfilePath + "/controls.sii", BindingType::wipers3, ets2BtnStr);
    }
}

BigKey ETS2KeyBinderWizard::getKeyState() {
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
        pushToQueue(parseErrorLog("设备poll()失败，错误代码：" + QString(std::to_string(HRESULT_CODE(hr)).data())));
        return keyState;
    }

    if (SUCCEEDED(pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
        // 获取按键状态
        for (int i = 0; i < capabilities.dwButtons; i++) {
            keyState.setBit(i, (js.rgbButtons[i] & 0x80));
        }
    } else {
        qDebug() << "获取设备状态信息失败!";
        qDebug() << "GetDeviceState failed with error:" << HRESULT_CODE(hr);
    }

    return keyState;
}
