#ifndef ASSISTFUNCWINDOW_H
#define ASSISTFUNCWINDOW_H

#include <QMainWindow>

#define ASSIST_FUNC_SETTINGS "assist_func_settings.json"

namespace Ui {
class AssistFuncWindow;
}

class AssistFuncWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AssistFuncWindow(QWidget *parent = nullptr);
    ~AssistFuncWindow();

signals:
    void stopWork();

private slots:
    void on_checkBox_stateChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::AssistFuncWindow *ui;

    QString appDataDirPath;// 软件数据存放路径

    bool ETS2_enbaleAutoCancelHandbreak = false;// 开启自动解除手刹

    void startAssistFuncWork();// 开启辅助功能任务

    void saveSettings();// 保存设置到文件
    void loadSettings();// 加载设置
};

#endif // ASSISTFUNCWINDOW_H
