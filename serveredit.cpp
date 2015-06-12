#include "serveredit.h"
#include "ui_serveredit.h"

serverEdit::serverEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serverEdit) {
    qDebug() << "serverEdit::serverEdit: constructor";
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags().operator ^=(Qt::WindowContextHelpButtonHint));

}

serverEdit::~serverEdit() {
    delete ui;
}

// Получение данных из главного окна
// и заполнение виджетов новой информацией
void serverEdit::recieveData(favServer server, QList<addon> addonsList, bool newServer, QStringList names) {

    qDebug() << "serverEdit::recieveData: start";

    newServ = newServer;
    // Настройка сервера
    //..при добавлении
    if(newServer) {
        // Установка параметров сервера
        ui->serverName->setText(tr("Новый сервер"));
        if(server.serverIP == "...")
            ui->serverIP->clear();
        else
            ui->serverIP->setText(server.serverIP);
        ui->serverPort->setText(server.serverPort);
        ui->serverPassword->setText(server.serverPass);
        ui->name_check->setChecked(false);
    // При зименении
    } else {
        // Установка параметров сервера
        ui->serverName->setText(server.serverName);
        ui->serverIP->setText(server.serverIP);
        ui->serverPort->setText(server.serverPort);
        ui->serverPassword->setText(server.serverPass);
        ui->name_check->setChecked(server.check_name);
    }

    // Заполнение списка аддонов
    ui->addonTree->clear();
    QTreeWidgetItem * item;
    for(int i=0; i<addonsList.count();i++) {
        // Добавление элемента
        item = new QTreeWidgetItem(ui->addonTree);
        item->setText(0, addonsList[i].addonName);
        item->setText(1, addonsList[i].addonDir);
        item->setText(2, addonsList[i].addonPath);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // Проверка, активен ли этот аддон на сервере
        QString fullPath = addonsList[i].addonPath + QString("/") +
                addonsList[i].addonDir;
        if(server.serverAddons.count()>0) {
            // Если содержит, то активируем
            if(server.serverAddons.contains(fullPath))
                item->setCheckState(0, Qt::Checked);
            else // Иначе. деактивируем
                item->setCheckState(0, Qt::Unchecked);
        } else { // Если у сервера нет подключенных аддонов, то сразу деактивируем
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    // Выбранный профиль сервера
    ui->name->clear();
    ui->name->addItems(names);
    ui->name->setCurrentText(server.name);

    this->open();
}

// Отправка измененных данных в главное окно
void serverEdit::on_save_clicked() {
    qDebug() << "serverEdit::on_save_clicked: Send start";

    // Создаем экземпляр класса favServer
    favServer server;
    // Заполняем его информацией из виджетов..
    //..параметров сервера
    server.serverName = ui->serverName->text();
    server.serverIP = ui->serverIP->text();
    server.serverPort = ui->serverPort->text();
    server.serverPass = ui->serverPassword->text();
    server.check_name = ui->name_check->isChecked();
    server.name = ui->name->currentText();

    //..Конвертируем Url в IP
    //..проверяем, IP ли это
    bool isIP = true;
    int pointCount = 0;
    int numberCount = 0;
    for(int i = 0; i< server.serverIP.size();i++) {
        if(server.serverIP[i].isNumber()) {
            isIP = true;
            numberCount++;
        } else if (server.serverIP[i] == '.') {
            isIP = true;
            pointCount++;
        } else {
            isIP = false;
            break;
        }
    }
    // Если это IP - проверяем его корректность
    if(isIP) { //127.0.0.1
        if(pointCount < 3 || numberCount <4 || numberCount > 12) {
            QMessageBox::warning(this,tr("Внимание!"), tr("Некорректный IP адресс сервера.\nВведите правильный IP сервера или его Url.\nПример: \"127.0.0.1\", \"example.server.com\"\nТип ошибки: IP Adress INCORRECT"), QMessageBox::Ok);
            qDebug() << "serverEdit::on_save_clicked: IP Adress INCORRECT";
            return;
        }
    // Если это Url - конвертируем в IP
    } else {
        QHostInfo info = QHostInfo::fromName(server.serverIP);
        if(info.errorString() == "Unknown error") {
            server.serverIP = info.addresses()[0].toString();
        } else {
            QMessageBox::warning(this,tr("Внимание!"), tr("Некорректный адресс сервера.\nВведите правильный IP сервера или его Url.\nПример: \"127.0.0.1\", \"example.server.com\"\nТип ошибки: ")+info.errorString(), QMessageBox::Ok);
            qDebug() << "serverEdit::on_save_clicked: " << info.errorString();
            return;
        }
    }

    //..активированных аддонов
    for(int i = 0;i<ui->addonTree->topLevelItemCount();i++)
        if(ui->addonTree->topLevelItem(i)->checkState(0) == Qt::Checked)
            server.serverAddons.append(ui->addonTree->topLevelItem(i)->text(2) + "/" +ui->addonTree->topLevelItem(i)->text(1));

    // Передаем экземпляр в сигнал
    emit sendData(server, newServ);
    this->close();
}

// Активирование аддонов по клику
void serverEdit::on_addonTree_itemClicked(QTreeWidgetItem *item) {

    if(addonTreeRow == ui->addonTree->currentIndex().row()) {
        if(item->isSelected()) {
            if(item->checkState(0) ==  Qt::Unchecked)
                item->setCheckState(0, Qt::Checked);
            else
                item->setCheckState(0, Qt::Unchecked);
        }
    }
    addonTreeRow = ui->addonTree->currentIndex().row();
}
