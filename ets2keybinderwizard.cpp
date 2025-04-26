#include "ets2keybinderwizard.h"
#include "global.h"
#include "ui_ets2keybinderwizard.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <map>
#include <regex>
#include <string>

// 参考开源项目：https://github.com/Sab1e-GitHub/ETS2-KeyBinder

using namespace std;

// 枚举类型定义
enum class BindingType
{
    lightoff,
    lighthorn,
    wipers0,
    wipers1,
    wipers2,
    wipers3,
    lightpark,
    lighton,
    hblight,
    lblinkerh,
    rblinkerh
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
    deviceNameInput = ui->comboBox->currentText().toStdString();
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

                if (hasLastDevInCurrentDeviceList(deviceNameInput)) {
                    ui->comboBox->setCurrentText(deviceNameInput.data());
                } else {
                    ui->comboBox->setCurrentIndex(-1);
                }
            }
        } else if (id == 3) {
            // 备份配置文件
            backupProfile();
        }
    });
}

ETS2KeyBinderWizard::~ETS2KeyBinderWizard() {
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
    deviceNameInput = ui->comboBox->currentText().toStdString();
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

// 1、示廓灯&近光灯
void ETS2KeyBinderWizard::on_pushButton_clicked() {}

// 2、远光灯&灯光喇叭
void ETS2KeyBinderWizard::on_pushButton_2_clicked() {}

// 3、左转向灯&右转向灯
void ETS2KeyBinderWizard::on_pushButton_3_clicked() {}

// 4、雨刮器
void ETS2KeyBinderWizard::on_pushButton_4_clicked() {}
