#pragma once

#include "qobject.h"
#include "qtmetamacros.h"
#include "ui/pages/about_page/AboutPage.h"
#include "ui/pages/forcefeedback_simulate_page/ForceFeedbackSimulatePage.h"
#include "ui/pages/home_page/HomePage.h"
#include "ui/pages/log_page/LogPage.h"
#include "ui/pages/settings_page/SettingsPage.h"
#include "utils/IconFactory.h"

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <windows.h>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QToolButton>
#include <QVector>


//
// 图标资源路径
//
struct IconResPath{
    // 常规图标-资源路径
    QString regularIconPath;
    // 填充图标-资源路径
    QString filledIconPath;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

protected:
    //////////////////////////////////////
    // 记录页面对象指针
    HomePage* homePage;
    SettingsPage* settingsPage;
    ForceFeedbackSimulatePage* forceFeedbackSimulatePage;
    LogPage* logPage;
    AboutPage* aboutPage;
    // 存放页面, 动态切换页面的容器
    QStackedWidget *stackedWidget;
    // 导航项列表
    QVector<QWidget*> navItems;
    // 导航项的图标 QLabel列表
    QVector<QLabel*> navIconLabels;
    // 导航项的图标枚举列表
    QVector<IconFactory::IconEnum> navIcons;
    // 导航项的图标大小
    const static inline int navIconSize = 28;

    //////////////////////////////////////
    // 绘制主窗口界面
    void paintMainWindowUI();
    // 创建导航项
    QWidget* createNavItem(const QString &text, IconFactory::IconEnum iconEnum);
    // 导航项的事件处理
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 设置当前选择的导航项
    void setActiveNav(int index);

    // 重写关闭窗口的事件
    void closeEvent(QCloseEvent *event) override;

private:
    //////////////////////////////////////////
    // 初始化
    void init();

    // 获取免费api LeanCloud 的访问域名
    void getApiHost(bool isSendUsage = true);
    // 记录软件启动日志
    void recordStartLog(QString loggerApi);
    // 检查软件是否有更新
    void checkUpdate(QString checkUpdateApi);

};

