#include "deleteotherfiles.h"
#include "ui_deleteotherfiles.h"

deleteOtherFiles::deleteOtherFiles(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::deleteOtherFiles) {
    qDebug() << "deleteOtherFiles::deleteOtherFiles: constructor";
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags().operator ^=(Qt::WindowContextHelpButtonHint));
}

deleteOtherFiles::~deleteOtherFiles() {
    delete ui;
}

// Получение данных о лишних файлах
void deleteOtherFiles::showDialog(const QList< QMap<QString, QString> > otherF, QString addonsP) {
    qDebug() << "deleteOtherFiles::showDialog: start";

    otherFiles = otherF;
    addonsPath = addonsP;

    ui->filesTree->clear();
    for(int i = 0; i<otherFiles.size();i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, otherFiles[i]["Path"]+'\\'+otherFiles[i]["Pbo"]);
        item->setCheckState(0, Qt::Checked);
    }

    this->open();
}

// Слот удаления лишних файлов
void deleteOtherFiles::on_deleteFiles_clicked() {
    qDebug() << "deleteOtherFiles::on_deleteFiles_clicked: start";

    for(int i = 0; i<ui->filesTree->topLevelItemCount();i++) {
        QTreeWidgetItem *item = ui->filesTree->topLevelItem(i);
        if(item->checkState(0) == Qt::Checked) {
            QFile::remove(addonsPath + "/" + item->text(0));
        }
    }

    emit filesDelete();
    this->close();
}
