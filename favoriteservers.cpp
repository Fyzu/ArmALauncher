#include "launcher.h"
#include "ui_launcher.h"

/*
 * Работа со списком избранных серверов
 */
// Добавить новый сервер
void launcher::on_serversTree_add_clicked() {

    qDebug() << "Debug-favServers: Add new server";

    // Получаем список аддонов
    addonsList.clear();
    for(int i = 0;i<ui->addonTree->topLevelItemCount();i++) {
        addonsList.append(addon(ui->addonTree->topLevelItem(i)->text(0), ui->addonTree->topLevelItem(i)->text(1), ui->addonTree->topLevelItem(i)->text(2)));
    }
    QStringList names;
    for(int i = 0;i<ui->name->count();i++)
        names.append(ui->name->itemText(i));
    // Получаем параметры сервера
    favServer server;
    server.serverIP = ui->serverIP->text();
    server.serverPort = ui->serverPort->text();
    server.serverPass = ui->serverPassword->text();

    //..активированных аддонов
    for(int i = 0;i<ui->addonTree->topLevelItemCount();i++)
        if(ui->addonTree->topLevelItem(i)->checkState(0) == Qt::Checked)
            server.serverAddons.append(ui->addonTree->topLevelItem(i)->text(2) + "/" +ui->addonTree->topLevelItem(i)->text(1));

    // Отправляем данные в форму
    emit sendData(server, addonsList, true, names);
}

// Удалить выбранный сервер
void launcher::on_serversTree_del_clicked() {
    if(favServers.count()>0) {  // Если нет элементов - нет смысла удалять
        int row = ui->serversTree->indexOfTopLevelItem(ui->serversTree->currentItem());
        if (row != -1) {   // Проверяем, выбран ли элемент
            qDebug() << "Debug-favServers: Del server - " << row;
            // Удаление сервера из памяти
            favServers.removeAt(ui->serversTree->currentItem()->data(0,Qt::UserRole).toUInt());
            // Обновляем информацию в виджетах после удаления в памяти
            updateInformationInWidget();
        }
    }
}

// Отправка информации о выбранном сервере
// в виджет окна редактирования и инициализация окна
void launcher::Send() {
    if(favServers.count()>0 && ui->serversTree->currentIndex().row() != -1) {   // Проверка, можно ли вызвать редактирование окна

        qDebug() << "Debug-favServers: Send data to serverEdit and Show Form";

        // Получаем список аддонов
        addonsList.clear();
        for(int i = 0;i<ui->addonTree->topLevelItemCount();i++) {
            addonsList.append(addon(ui->addonTree->topLevelItem(i)->text(0), ui->addonTree->topLevelItem(i)->text(1), ui->addonTree->topLevelItem(i)->text(2)));
        }
        QStringList names;
        for(int i = 0;i<ui->name->count();i++)
            names.append(ui->name->itemText(i));
        // Отправляем информацию о выбранном сервере
        selectServer = ui->serversTree->currentItem()->data(0,Qt::UserRole).toUInt();
        emit sendData(favServers[selectServer], addonsList, false, names);

        qDebug() << "Debug-favServers: Send data to serverEdit and Show Form - succ";
    }
}

// Получение данных из окна редактирования сервера
void launcher::recieveData(favServer server, bool newServer) {

    if(newServer) {
        // Добавляеем новый сервер в память
        favServers.append(server);
    } else {
        // Обновляем данные о выбранном сервере в памяти
        favServers[selectServer] = server;
    }
    // Обновляем данные о серверах в виджете
    updateInformationInWidget();
    qDebug() << "Debug-favServers: Recieve data from serverEdit - succ";
}

// При смене сервера для запуска
// Активируется слот включения нужных аддонов для сервера
void launcher::on_selectServer_currentIndexChanged(int index) {

    if (!updateInfoInWidget) {  // Для исключения вызова слота при обновлении информации в виджетах
        if(index != 0) {          // Исключить выбор несуществующего элемента

            qDebug() << "Debug-favServers: Upd. Inf. in Addon Tree, Server select - " << index;
            //Итерируем список аддонов, где i - индекс аддона в списке
            for (int i = 0; i<ui->addonTree->topLevelItemCount();i++) {
                QString fullPath = ui->addonTree->topLevelItem(i)->text(2) + QString("/") +
                        ui->addonTree->topLevelItem(i)->text(1);
                if(favServers[index-1].serverAddons.count()>0) // Есть ли вообще аддоны у этого сервера
                    if(favServers[index-1].serverAddons.contains(fullPath)) // Если находим такой аддон в активированных у сервера
                        ui->addonTree->topLevelItem(i)->setCheckState(0, Qt::Checked);
                    else
                        ui->addonTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
                else
                    ui->addonTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
            }

            // Заполняем данные о сервере
            ui->serverIP->setText(favServers[index-1].serverIP);
            ui->serverPort->setText(favServers[index-1].serverPort);
            ui->serverPassword->setText(favServers[index-1].serverPass);

        } else {
            // Очищаем поля
            ui->serverIP->clear();
            ui->serverPort->clear();
            ui->serverPassword->clear();
        }
        // Обновляем параметры запуска
        updateInfoInRunParametersWidget();

    }
}

// Получение подробной информации о сервере
/*void launcher::on_serversTree_about_clicked() {

}*/

// Обновление онлайн информации всех серверов
void launcher::on_serversTree_update_clicked() {

    // Начинаем обновление данных серверов
    if(favServers.count() == 0) return;     // Если серверов нет
    qDebug() << "Debug-favServers: Upd. Online server Infomation";

    // Переменные для работы
    int server;                 // Индекс сервера, о котором получают информацию
                                // Сообщение "TSource query"
    unsigned char message[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45,
                               0x6E, 0x67, 0x69, 0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00};
    int length = 25;            // Длина отправляемой строки
    int ping;                   // Переменная для хранения значения пинга сервера

    // Отключаем сортировку serversTree
    // (потому что прописываем новые названия, сортировка меняет индексацию)
    ui->serversTree->setSortingEnabled(false);

    // Обновляем каждый сервер по отдельности
    for(int i = 0; i < ui->serversTree->topLevelItemCount();i++) {

        // Получаем индекс информации сервера в БД
        server = ui->serversTree->topLevelItem(i)->data(0,Qt::UserRole).toUInt();
        qDebug() << "Debug-favServers: Upd. Server - index - " << server;

        // Проверяем IP и Port на корректность
        bool correct=true;
        for (int index = 0; index<favServers[server].serverIP.count();index++)
            if(!favServers[server].serverIP[index].isNumber() && favServers[server].serverIP[index] != '.')
                correct=false;
        for(int index = 0; index<favServers[server].serverPort.count();index++)
            if(!favServers[server].serverPort[index].isNumber())
                correct=false;
        if(favServers[server].serverIP.isEmpty() || favServers[server].serverPort.isEmpty() || !correct) {
            qDebug() << "Debug-favServers: Upd. Server - fail - index - " << server;
            continue;
        }
        /*
        unsigned char testMess[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0x4B, 0xA1, 0xD5, 0x22};
        qDebug() << testMess;
        //{0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0x4B, 0xA1, 0xD5, 0x22};
        //{0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0xFF};
        BYTE* test = exchangeDataWithServer(QString("37.187.170.178").toLatin1().data(), QString("2302").toInt()+1, 400, testMess,9, ping);
        int inde = 0;
        qDebug() << test;
        testMess[5] = test[5];
        testMess[6] = test[6];
        testMess[7] = test[7];
        testMess[8] = test[8];
        qDebug() << testMess;
        test = exchangeDataWithServer(QString("37.187.170.178").toLatin1().data(), QString("2302").toInt()+1, 400, testMess,9, ping);
        qDebug() << test;
        for(int b = 0;b<200;b++)
        qDebug() << GetNextPart(inde, test);*/
        // Запуск инструмента получения информации о сервере
        BYTE* response = exchangeDataWithServer(favServers[server].serverIP.toLatin1().data(), favServers[server].serverPort.toInt()+1, 400, message, length, ping);
        // Парсим ответ сервера
        if(ping>0) {                // Если получен ответ от сервера
            int pos = 6;                                                            // Позиция начала строки
            favServers[server].ping = QString::number(ping);                        // Прописываем пинг
            favServers[server].HostName = QString(GetNextPart(pos, response));      // Прописываем имя сервера
            favServers[server].MapName = QString(GetNextPart(pos, response));       // Прописываем название карты
            favServers[server].GameType = QString(GetNextPart(pos, response));      // Прописываем тип игры
            favServers[server].GameMode = QString(GetNextPart(pos, response));      // Прописываем мод игры
            GetNextPart(pos, response);                                             // Пустая строка
            GetNextPart(pos, response);                                             // Пустая строка
            QByteArray pInfo = GetNextPart(pos, response);                          // Получаем кол-во игроков
            if((int)pInfo[0] >= 0 && (int)pInfo[1] >= 0) {
                if(pInfo.length()>0) {
                    favServers[server].NumPlayers=QString::number((int)pInfo[0]);
                    favServers[server].MaxPlayers=QString::number((int)pInfo[1]);
                } else {
                    favServers[server].NumPlayers="0";
                    favServers[server].MaxPlayers=QString::number((int)GetNextPart(pos, response)[0]);
                }
            } else {
                favServers[server].NumPlayers="-";
                favServers[server].MaxPlayers="-";
            }
        } else {                    // Если не получен ответ от сервера
            favServers[server].noResponse();
        }

        // Устанавливаем обновленную онлайн информацию о серверах
        QIcon icon;
        if(!favServers.at(server).ping.isEmpty()) {     // Если пинг есть, то присваиваем онлайн информацию
            ui->serversTree->topLevelItem(i)->setText(0, favServers[server].HostName);
            ui->selectServer->setItemText(server+1, favServers[server].HostName);
            ui->serversTree->topLevelItem(i)->setText(4, favServers[server].NumPlayers + "/" + favServers[server].MaxPlayers);
            ui->serversTree->topLevelItem(i)->setText(5, favServers[server].ping);
            icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->serversTree->topLevelItem(i)->setIcon(5, icon);
            ui->selectServer->setItemIcon(server+1, icon);
        }
        else { // Если не пингуется, то присваиваем стандартную информацию
            ui->serversTree->topLevelItem(i)->setText(0, favServers[server].serverName);
            ui->selectServer->setItemText(server+1, favServers[server].serverName);
            ui->serversTree->topLevelItem(i)->setText(4, "-/-");
            ui->serversTree->topLevelItem(i)->setText(5, " - ");
            icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->serversTree->topLevelItem(i)->setIcon(5, icon);
            ui->selectServer->setItemIcon(server+1, icon);
        }
    }

    qDebug() << "Debug-favServers: Upd. Online server Infomation - succ";
    // Возвращаем сортировку серверов
    ui->serversTree->setSortingEnabled(true);
}

// Выделение подстрок в байтовом массиве
QByteArray launcher::GetNextPart(int &pos, BYTE* response) {
    QByteArray part;
    for (pos = pos; response[pos] != 0x0; pos++) {
        part.append(response[pos]);
    }
    pos++;
    return part;
}
