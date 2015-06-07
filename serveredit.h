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

public slots:
    // Получение данных от главного окна
    void recieveData(favServer server, QList<addon> addonsList, bool newServer, QStringList names);

private slots:
    // Подготовка передачи данных в главное окно
    void on_save_clicked();

    void on_addonTree_itemClicked(QTreeWidgetItem *item);

signals:
    // Передача данных в главное окно
    void sendData(favServer server, bool newServer);

private:
    Ui::serverEdit *ui;
    bool newServ;
};

#endif // SERVEREDIT_H
