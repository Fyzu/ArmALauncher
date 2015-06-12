#ifndef REPOEDIT_H
#define REPOEDIT_H

#include <QDialog>
#include "serveredit.h"
#include <QMessageBox>

#include "launcherupdate.h"

namespace Ui {
    class repoEdit;
}

class repoEdit : public QDialog
{
    Q_OBJECT

public:
    explicit repoEdit(QWidget *parent = 0);
    ~repoEdit();
signals:
    void sendData(Repository repo, int currentRow, bool newRepo);
private slots:
    void recieveData(Repository repo, int currentRow, bool newRepo);
    void on_repoType_currentIndexChanged(int index);
    void on_saveButton_clicked();
private:
    Ui::repoEdit *ui;
    bool newR;
    int Row;
};

#endif // REPOEDIT_H
