#ifndef SERVEREDIT_H
#define SERVEREDIT_H

#include <QDialog>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QHostInfo>
#include <QMessageBox>
#include <QList>

#include "updateaddons.h"

namespace Ui {
    class serverEdit;
}

class serverEdit : public QDialog
{
    Q_OBJECT

public:
    explicit serverEdit(QWidget *parent = 0);
    ~serverEdit();
private slots:
    void recieveData(favServer server, QList<addon> addonsList, bool newServer, QStringList names);
    void on_save_clicked();
    void on_addonTree_itemClicked(QTreeWidgetItem *item);
signals:
    void sendData(favServer server, bool newServer);
private:
    Ui::serverEdit *ui;
    bool newServ;
    int addonTreeRow;
};

#endif // SERVEREDIT_H
