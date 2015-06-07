#ifndef ADDONSSETTINGS_H
#define ADDONSSETTINGS_H

#include <QDialog>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QListWidget>

#include "repoedit.h"

namespace Ui {
    class addonsSettings;
}

class addonsSettings : public QDialog
{
    Q_OBJECT

public:
    explicit addonsSettings(QWidget *parent = 0);
    ~addonsSettings();
signals:
    void sendData(QStringList listD, QStringList listPriorityAddonsD);

public slots:
    void receiveData(Settings settings, QStringList listD, QStringList listPriorityAddonsD, QStringList addons);

private slots:
    void on_addonSearchDirectories_add_clicked();
    void on_addonSearchDirectories_del_clicked();
    void on_addonsPriorities_up_clicked();
    void on_addonsPriorities_down_clicked();
    void on_buttonBox_accepted();

private:
    Ui::addonsSettings *ui;

    bool ListWidgetContains(QListWidget * widget, QString str);

    QStringList listDirs;
    QStringList listPriorityAddonsDirs;
};

#endif // ADDONSSETTINGS_H
