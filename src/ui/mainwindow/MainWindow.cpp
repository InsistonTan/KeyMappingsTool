#include "ui/mainwindow/MainWindow.h"
#include "qdatetime.h"
#include "qpixmap.h"
#include "common/StringConstants.h"
#include "common/Theme.h"
#include "services/ConfigService.h"
#include "services/DirectInputService.h"
#include "services/LogService.h"
#include "ui/pages/forcefeedback_simulate_page/ForceFeedbackSimulatePage.h"
#include "ui/pages/about_page/AboutPage.h"
#include "ui/pages/home_page/HomePage.h"
#include "ui/pages/log_page/LogPage.h"
#include "ui/pages/settings_page/SettingsPage.h"
#include "utils/IconFactory.h"
#include "common/Global.h"

#include<QApplication>
#include<QMainWindow>
#include<QFrame>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QPushButton>
#include<QLabel>
#include<QComboBox>
#include<QTableView>
#include<QStandardItemModel>
#include<QHeaderView>
#include<QStatusBar>
#include<QLabel>
#include<QVBoxLayout>
#include<QKeyEvent>
#include<QDebug>
#include<QLineEdit>
#include<QFile>
#include<QInputDialog>
#include<QDir>
#include<QSettings>
#include<QProcess>
#include<QCheckBox>
#include<QTimer>
#include<QScrollBar>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QNetworkAccessManager>
#include<QNetworkRequest>
#include<QNetworkReply>
#include<QRegularExpression>
#include<QHostInfo>
#include<QDate>
#include<QDesktopServices>
#include<QUrl>
#include<QStyleHints>
#include<QTextEdit>
#include<QSet>
#include <QDialogButtonBox>


MainWindow::~MainWindow(){
    DirectInputService::cleanupDirectInput();
}

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
{
    // 默认窗口大小
    this->resize(1000, 650);
    this->setWindowTitle(Global::APP_NAME);
    this->setWindowIcon(QIcon(":/app.ico"));

    // 绘制主窗口界面UI
    paintMainWindowUI();

    // 当前exe程序所在路径
    QString exePath = QCoreApplication::applicationDirPath();

    if(exePath.toLower().contains("build") && (exePath.toLower().contains("release") || exePath.toLower().contains("debug"))){
        qDebug() << "当前为开发环境, 不记录日志";
    }else{
        QTimer::singleShot(0,this, [this](){
            // 记录启动日志
            recordStartLog(Global::LOGGER_API);
            // 检查更新
            checkUpdate(Global::CHECK_UPDATE_API);
        });
    }
}

void MainWindow::init(){
    // 日志窗口
    //this->logWindow = new LogWindow();

    // 初始化死区设置窗口
    //this->deadareaSettings = new DeadAreaSettings();

    // 初始化辅助功能窗口, 加载辅助功能设置
    //this->assistWindow = new AssistFuncWindow();
}

void MainWindow::recordStartLog(QString loggerApi){
    if(loggerApi.isEmpty()){
        return;
    }

    // 发送软件使用情况
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    // 设置请求 URL
    QUrl url(loggerApi);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 获取主机名
    QString username = QHostInfo::localHostName();

    // 构造 JSON 数据
    QJsonObject json;
    json["log_id"] =
        QString::number(QDateTime::currentMSecsSinceEpoch())
        + "_"
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    json["app"] = Global::APP_NAME;
    json["username"] = username.isEmpty() ? "unknown" : username;
    json["time"] = QDateTime::currentDateTime().toString("yyyyMMdd_HH:mm:ss");
    json["version"] = Global::CURRENT_VERSION;

    QByteArray data = QJsonDocument(json).toJson();
    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, data);

    // 连接信号槽处理响应
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray response = reply->readAll();
        QString responseStr = QString(response);

        qDebug() << responseStr;

        reply->deleteLater();
        manager->deleteLater();
    });
}

void MainWindow::checkUpdate(QString checkUpdateApi){
    // 设置默认的api域名
    if(checkUpdateApi.isEmpty()){
        return;
    }

    // 检查软件更新
    QNetworkAccessManager *manager2 = new QNetworkAccessManager();
    // 设置请求 URL
    QUrl url2(checkUpdateApi);
    QNetworkRequest request2(url2);

    // 发送 GET 请求
    QNetworkReply *reply2 = manager2->get(request2);

    // 连接信号槽处理响应
    QObject::connect(reply2, &QNetworkReply::finished, [=]() {
        if (reply2->error() == QNetworkReply::NoError) {
            QByteArray response = reply2->readAll();
            QString responseStr = QString(response);


            QString key_latest_version = "latest_version";
            QString key_release_note = "release_note";
            QString key_download_github = "download_github";
            QString key_download_lanzou = "download_lanzou";

            if(responseStr.contains(key_latest_version)){
                QJsonDocument doc = QJsonDocument::fromJson(response);
                if(!doc.isNull() && doc.isObject()){
                    auto obj = doc.object();

                    // 是否需要更新
                    bool needUpdate = (obj[key_latest_version].toString() > Global::CURRENT_VERSION);

                    if(needUpdate){
                        // 更新日志
                        QString releaseNote = (obj.contains(key_release_note) && !obj[key_release_note].toString().isEmpty()
                                                  ? obj[key_release_note].toString()
                                                  : StringConstants::haveNotReleaseNote);

                        releaseNote.replace("\n", "<br>");
                        releaseNote.replace(" ", "&nbsp;");

                        QString text = "<b style='color:#22C55E;font-size:14px;'>"+StringConstants::newVersion+": v" + obj[key_latest_version].toString() + "</b>"
                                       + "<br><br>"
                                       + "<b>" + StringConstants::releaseNote + "</b>:<br>"
                                       + releaseNote;

                        QDialog dialog;
                        Theme::setQWidgetStyleSheet(&dialog);
                        dialog.setWindowTitle(StringConstants::versionUpdateTips);
                        dialog.setMinimumSize(500, 400);
                        QVBoxLayout *layout = new QVBoxLayout(&dialog);

                        QTextEdit *textEdit = new QTextEdit();
                        Theme::setScrollBarStyleSheet(textEdit);
                        textEdit->setText(text);
                        textEdit->setReadOnly(true);

                        QDialogButtonBox *buttonBox = new QDialogButtonBox();

                        // 添加github下载按钮
                        QPushButton *githubButton = new QPushButton(StringConstants::gitHubDownload);
                        Theme::setButtonStyleSheet(githubButton, ButtonLevel::primary);
                        buttonBox->addButton(githubButton, QDialogButtonBox::ActionRole);
                        connect(githubButton, &QPushButton::clicked, [&](){
                            QDesktopServices::openUrl(QUrl(obj.contains(key_download_github) && !obj[key_download_github].toString().isEmpty()
                                                               ? obj[key_download_github].toString()
                                                               : ""));
                        });

                        // 添加蓝奏云下载按钮
                        QPushButton *lanzouButton = new QPushButton(StringConstants::lanzouDownload);
                        Theme::setButtonStyleSheet(lanzouButton, ButtonLevel::primary);
                        buttonBox->addButton(lanzouButton, QDialogButtonBox::ActionRole);
                        connect(lanzouButton, &QPushButton::clicked, [&](){
                            QDesktopServices::openUrl(QUrl(obj.contains(key_download_lanzou) && !obj[key_download_lanzou].toString().isEmpty()
                                                               ? obj[key_download_lanzou].toString()
                                                               : ""));
                        });

                        // 添加取消按钮
                        QPushButton *cancelButton = new QPushButton(StringConstants::btnText_cancel);
                        Theme::setButtonStyleSheet(cancelButton, ButtonLevel::normal);
                        buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
                        connect(cancelButton, &QPushButton::clicked, [&](){
                            dialog.close();
                        });

                        layout->addWidget(textEdit);
                        layout->addWidget(buttonBox);

                        dialog.exec();
                    }
                }
            }
        } else {
            qDebug() << "Http Request Error:" << reply2->errorString();
            LogService::parseWarningLog(StringConstants::checkLatestVersionFailed.arg(checkUpdateApi, reply2->errorString()));
        }
        reply2->deleteLater();
        manager2->deleteLater();
    });
}

void MainWindow::paintMainWindowUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // #0D121D
    central->setStyleSheet(QString(R"(
    QWidget {
        background-color: %1;
    }
    )").arg(Theme::rootBg()));


    QHBoxLayout *root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    // =========================
    // 左侧导航
    // =========================
    QFrame *sidebar = new QFrame;
    sidebar->setFixedWidth(160);
    sidebar->setStyleSheet(QString(R"(
        QFrame{
            background: %1;
            border-radius: 0px;
        }
    )").arg(Theme::panelBg()));

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);


    // 添加左侧导航栏项
    sideLayout->addWidget(createNavItem(StringConstants::keyMapping, IconFactory::IconEnum::keymapping));
    sideLayout->addWidget(createNavItem(StringConstants::ffbSimulate, IconFactory::IconEnum::ffb_simulate));
    sideLayout->addWidget(createNavItem(StringConstants::log, IconFactory::IconEnum::log));
    sideLayout->addWidget(createNavItem(StringConstants::settings, IconFactory::IconEnum::settings));
    sideLayout->addStretch();
    sideLayout->addWidget(createNavItem(StringConstants::about, IconFactory::IconEnum::support));


    // =========================
    // 右侧 QStackedWidget
    // =========================
    stackedWidget = new QStackedWidget();

    // 添加页面
    homePage = new HomePage();
    forceFeedbackSimulatePage = new ForceFeedbackSimulatePage();
    logPage = new LogPage();
    settingsPage = new SettingsPage();
    aboutPage = new AboutPage();

    // 绑定 用户设备改变的信号 到 homePage 的槽函数
    connect(settingsPage, &SettingsPage::settingsChanged, homePage, &HomePage::settingsChangedSlot);
    connect(forceFeedbackSimulatePage, &ForceFeedbackSimulatePage::settingsChanged, homePage, &HomePage::settingsChangedSlot);
    // 绑定 当前选择的映射配置改变 信号
    connect(homePage, &HomePage::currentSelectedMappingFileChanged, forceFeedbackSimulatePage, &ForceFeedbackSimulatePage::currentSelectedMappingFileChangedSlot);
    connect(homePage, &HomePage::currentSelectedMappingFileChanged, settingsPage, &SettingsPage::currentSelectedMappingFileChangedSlot);

    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(forceFeedbackSimulatePage);
    stackedWidget->addWidget(logPage);
    stackedWidget->addWidget(settingsPage);
    stackedWidget->addWidget(aboutPage);

    // 默认选中导航栏首位
    setActiveNav(0);

    root->addWidget(sidebar);
    root->addWidget(stackedWidget, 1);
}

QWidget* MainWindow::createNavItem(const QString &text, IconFactory::IconEnum iconEnum)
{
    QWidget *item = new QWidget;
    item->setFixedHeight(40);
    item->setObjectName("QWidget");

    QHBoxLayout *layout = new QHBoxLayout(item);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(12);

    // =========================
    // 左侧蓝色指示条
    // =========================
    QFrame *indicator = new QFrame;
    indicator->setFixedWidth(3);
    indicator->setStyleSheet("background:transparent;");

    // ======================
    // svg icon
    // ======================
    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(navIconSize, navIconSize);
    iconLabel->setScaledContents(true);
    iconLabel->setStyleSheet("background:transparent;");

    // =========================
    // text
    // =========================
    QLabel *textLabel = new QLabel(text);
    textLabel->setStyleSheet(QString("background:transparent;color:%1;")
                                 .arg(Theme::textColor()));

    layout->addWidget(indicator);
    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addStretch();

    // 点击事件（关键）
    item->installEventFilter(this);

    // 保存导航项
    navItems.push_back(item);
    // 保存指示条
    //indicators.push_back(indicator);
    // 保存导航项图标
    navIconLabels.push_back(iconLabel);
    // 保存图标枚举
    navIcons.push_back(iconEnum);

    return item;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        for (int i = 0; i < navItems.size(); i++)
        {
            if (navItems[i] == obj)
            {
                setActiveNav(i);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setActiveNav(int index)
{
    for (int i = 0; i < navItems.size(); i++)
    {
        QWidget* item = navItems[i];
        QLabel* iconLabel = navIconLabels[i];
        auto iconEnum = navIcons[i];

        if (i == index)
        {
            item->setStyleSheet(R"(
                QWidget#QWidget { background: transparent; border-radius: 12px; border:1px solid #3B82F6;}
                QWidget#QWidget:hover { background: transparent; }
            )");

            iconLabel->setPixmap(IconFactory::icon(iconEnum, navIconSize).pixmap(navIconSize, navIconSize));
        }
        else
        {
            // 1f2937
            item->setStyleSheet(R"(
                QWidget#QWidget { background: transparent; border-radius: 12px;border:none;}
                QWidget#QWidget:hover { background: transparent; border:1px solid #4F5967;}
            )");

            // 显示图标 "#acc3cf"
            iconLabel->setPixmap(IconFactory::colorIcon(iconEnum, QColor("#acc3cf"), navIconSize).pixmap(navIconSize, navIconSize));
        }
    }

    stackedWidget->setCurrentIndex(index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(ConfigService::getGlobalUserConfig().SYSTEM_enableHideWindowOnClose){
        // 隐藏窗口
        this->hide();
        // 阻止事件
        event->ignore();
    }
}


