#ifndef DEBBIN_H
#define DEBBIN_H

#include <QMainWindow>

namespace Ui {
    class debbin;
}

class debbin : public QMainWindow
{
    Q_OBJECT

public:
    explicit debbin(QWidget *parent = 0);
    ~debbin();

private:
    Ui::debbin *ui;
    QString s_currentConfig;
    void readConfig(const QString file);
private slots:
    void on_btn_add_clicked();
    void on_actionAbout_triggered();
    void on_actionCreate_package_triggered();
    void on_actionGenerate_control_file_triggered();
    void on_actionQuit_triggered();
    void on_btn_outbutdir_clicked();
    void on_btn_filesystem_clicked();
    void on_btn_createpackage_clicked();
    void on_pushButton_clicked();
    void on_checkBox_toggled(bool checked);
    void on_actionUser_manual_triggered();
    void on_actionClear_triggered();
    void on_readyRead();
    void on_action_Open_Config_triggered();
    void on_saveConfigButton_clicked();
    void on_action_Save_Config_triggered();
    void on_cmb_condition_activated(const QString &arg1);
    void on_delButton_clicked();
};

#endif // DEBBIN_H
