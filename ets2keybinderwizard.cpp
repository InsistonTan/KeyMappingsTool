#include "ets2keybinderwizard.h"
#include "BigKey.hpp"
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

QString LOG_HEADER = "<b>特殊按键绑定</b> - ";
QString MEG_BOX_LINE = "------------------------";

// 参考开源项目：https://github.com/Sab1e-GitHub/ETS2-KeyBinder

using namespace std;

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
void modify_controls_sii(const QString& controlsFilePath, BindingType bindingType, const QString& ets2Str) {
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

    if (replaceRules.find(bindingType) == replaceRules.end()) {
        qWarning() << "无效的绑定类型";
        return;
    }

    // 获取替换规则
    QString pattern = replaceRules[bindingType];
    QString replacement = QString("mix %1 `%2 | semantical.%1?0`").arg(replaceRules[bindingType]).arg(ets2Str);

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

// 1、示廓灯&近光灯
void ETS2KeyBinderWizard::on_pushButton_clicked() {
    int stepSum = 3; // 步骤总数
    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "示廓灯&近光灯：1/" + QString::number(stepSum), "请将拨杆拧到：\n" + MEG_BOX_LINE + "\n关闭灯光");
    box.exec();
    // 2、将拨杆拨到示廓灯位置
    box.setWindowTitle("示廓灯&近光灯：2/" + QString::number(stepSum));
    box.setText("请将拨杆拧到：\n" + MEG_BOX_LINE + "\n示廓灯");
    box.exec();

    // 3、将拨杆拨到近光灯位置
    box.setWindowTitle("示廓灯&近光灯：3/" + QString::number(stepSum));
    box.setText("请将拨杆拧到：\n" + MEG_BOX_LINE + "\n近光灯");
    box.exec();
    // 4、确定是否绑定
    box.setWindowTitle("示廓灯&近光灯");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);

    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // 绑定操作
        qDebug() << "绑定操作";
    } else {
        // 取消绑定操作
        qDebug() << "取消绑定操作";
    }
}

// 2、远光灯&灯光喇叭
void ETS2KeyBinderWizard::on_pushButton_2_clicked() {
    int stepSum = 3; // 步骤总数
    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "远光灯&灯光喇叭：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    // 2、将拨杆拨到远光灯位置
    box.setWindowTitle("远光灯&灯光喇叭：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n远光灯");
    box.exec();
    // 3、将拨杆拨到灯光喇叭位置
    box.setWindowTitle("远光灯&灯光喇叭：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n灯光喇叭");
    box.exec();
    // 4、生成配置文件
    box.setWindowTitle("远光灯&灯光喇叭");
    box.setText("欧卡不支持远光灯的同步绑定，已生成配置文件，请回到主界面后用此配置文件开启全局映射。");
    box.exec();
}

// 3、左转向灯&右转向灯
void ETS2KeyBinderWizard::on_pushButton_3_clicked() {
    int stepSum = 3; // 步骤总数
    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "左转向灯&右转向灯：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    // 2、将拨杆拨到左转向灯位置
    box.setWindowTitle("左转向灯&右转向灯：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n左转向灯");
    box.exec();
    // 3、将拨杆拨到右转向灯位置
    box.setWindowTitle("左转向灯&右转向灯：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到：\n" + MEG_BOX_LINE + "\n右转向灯");
    box.exec();
    // 4、确定是否绑定
    box.setWindowTitle("左转向灯&右转向灯");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);
    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // 绑定操作
        qDebug() << "绑定操作";
    } else {
        // 取消绑定操作
        qDebug() << "取消绑定操作";
    }
}

// 4、雨刮器
void ETS2KeyBinderWizard::on_pushButton_4_clicked() {
    int stepSum = 4; // 步骤总数
    // 1、将拨杆拨到关闭位置
    QMessageBox box(QMessageBox::Information, "雨刮器：1/" + QString::number(stepSum), "请将拨杆拨到：\n" + MEG_BOX_LINE + "\n关闭位置");
    box.exec();
    // 2、将拨杆拨到雨刮器1档位置
    box.setWindowTitle("雨刮器：2/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器1档");
    box.exec();
    // 3、将拨杆拨到雨刮器2档位置
    box.setWindowTitle("雨刮器：3/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器2档");
    box.exec();
    // 4、将拨杆拨到雨刮器3档位置
    box.setWindowTitle("雨刮器：4/" + QString::number(stepSum));
    box.setText("请将拨杆拨到\n" + MEG_BOX_LINE + "\n雨刮器3档");
    box.exec();
    // 5、确定是否绑定
    box.setWindowTitle("雨刮器 绑定");
    box.setText("是否绑定？");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);
    int ret = box.exec();
    if (ret == QMessageBox::Yes) {
        // 绑定操作
        qDebug() << "绑定操作";
    } else {
        // 取消绑定操作
        qDebug() << "取消绑定操作";
    }
}
