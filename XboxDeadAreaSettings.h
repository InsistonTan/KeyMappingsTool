#ifndef XBOXDEADAREASETTINGS_H
#define XBOXDEADAREASETTINGS_H

#include <QMainWindow>

#define SETTINGS_FILE_NAME "xbox_settings.json"

namespace Ui {
class XboxDeadAreaSettings;
}

class XboxDeadAreaSettings : public QMainWindow
{
    Q_OBJECT

public:
    explicit XboxDeadAreaSettings(QWidget *parent = nullptr);
    ~XboxDeadAreaSettings();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_editingFinished();

    void on_pushButton_clicked();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_2_textChanged(const QString &arg1);

private:
    Ui::XboxDeadAreaSettings *ui;

    void unsave();
    void save();

    void saveSettingsToFile();
    void loadSettingsFile();
};

#endif // XBOXDEADAREASETTINGS_H
