#ifndef DELETEOTHERFILES_H
#define DELETEOTHERFILES_H

#include <QDialog>
#include <QStringList>
#include <QFile>
#include <QMap>

#include "xmlparser.h"

namespace Ui {
    class deleteOtherFiles;
}

class deleteOtherFiles : public QDialog
{
    Q_OBJECT

public:
    explicit deleteOtherFiles(QWidget *parent = 0);
    ~deleteOtherFiles();
signals:

    void filesDelete();
public slots:
    void showDialog(const QList< QMap<QString, QString> > otherF, QString addonsP);

private slots:
    void on_deleteFiles_clicked();

private:
    Ui::deleteOtherFiles *ui;

    QList< QMap<QString, QString> > otherFiles;
    QString addonsPath;
};

#endif // DELETEOTHERFILES_H
