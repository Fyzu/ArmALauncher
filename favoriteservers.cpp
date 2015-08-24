#include "launcher.h"
#include "ui_launcher.h"

/*
 * Работа со списком избранных серверов
 */
// Добавить новый сервер
void launcher::on_serversTree_add_clicked() {

    qInfo() << "launcher::on_serversTree_add_clicked: Add new server";

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

    if(!timerCheckServerStatus->isActive()) {
        if(favServers.count()>0) {  // Если нет элементов - нет смысла удалять
            int row = ui->serversTree->currentIndex().row();
            if (row != -1) {   // Проверяем, выбран ли элемент
                qInfo() << "launcher::on_serversTree_del_clicked: Del server -" << row;
                // Удаление сервера из памяти
                favServers.removeAt(ui->serversTree->currentItem()->data(0,Qt::UserRole).toUInt());
                // Обновляем информацию в виджетах после удаления в памяти
                updateInformationInWidget();
            }
        }
    } else {
        QMessageBox::warning(this,tr("Внимание!"), tr("Запрещено удалять сервера пока запущено слежение за сервером.\nОстановите слежение за сервером, перед удалением сервера."), QMessageBox::Ok);
        qWarning() << "launcher::on_serversTree_del_clicked: error, monitoring is running";
    }
}

// Отправка информации о выбранном сервере
// в виджет окна редактирования и инициализация окна
void launcher::Send() {

    if(favServers.count()>0 && ui->serversTree->currentIndex().row() != -1) {   // Проверка, можно ли вызвать редактирование окна

        qInfo() << "launcher::Send: Send data to serverEdit and Show Form";

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

    }
}

// Получение данных из окна редактирования сервера
void launcher::recieveData(favServer server, bool newServer) {

    qInfo() << "launcher::recieveData: Recieve data from serverEdit";

    if(newServer) {
        // Добавляеем новый сервер в память
        favServers.append(server);
    } else {
        // Обновляем данные о выбранном сервере в памяти
        favServers[selectServer] = server;
    }
    // Обновляем данные о серверах в виджете
    updateInformationInWidget();
}

// При смене сервера для запуска
// Активируется слот включения нужных аддонов для сервера
void launcher::on_selectServer_currentIndexChanged(int index) {

    if (!updateInfoInWidget && index >= 0) {  // Для исключения вызова слота при обновлении информации в виджетах
        if(index != 0) {          // Исключить выбор несуществующего элемента

            qInfo() << "launcher::on_selectServer_currentIndexChanged: Upd. Inf. in Addon Tree, Server select -" << index;
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
    qInfo() << "launcher::on_serversTree_update_clicked: Upd. Online servers Infomation";

    // Переменные для работы
    int serverIndex;                 // Индекс сервера, о котором получают информацию

    // Отключаем сортировку serversTree
    // (потому что прописываем новые названия, сортировка меняет индексацию)
    ui->serversTree->setSortingEnabled(false);

    // Обновляем каждый сервер по отдельности
    for(int i = 0; i < ui->serversTree->topLevelItemCount();i++) {

        // Получаем индекс информации сервера в БД
        serverIndex = ui->serversTree->topLevelItem(i)->data(0,Qt::UserRole).toUInt();
        qInfo() << "launcher::on_serversTree_update_clicked: Upd. Server - index -" << serverIndex;

        // Проверяем IP и Port на корректность
        bool correct=true;
        for (int index = 0; index<favServers[serverIndex].serverIP.count();index++)
            if(!favServers[serverIndex].serverIP[index].isNumber() && favServers[serverIndex].serverIP[index] != '.')
                correct=false;
        for(int index = 0; index<favServers[serverIndex].serverPort.count();index++)
            if(!favServers[serverIndex].serverPort[index].isNumber())
                correct=false;
        if(favServers[serverIndex].serverIP.isEmpty() || favServers[serverIndex].serverPort.isEmpty() || !correct) {
            qInfo() << "launcher::favServers: Upd. Server - fail - index -" << serverIndex;
            continue;
        }

        updateServerInformation(serverIndex, i);

    }
    // Возвращаем сортировку серверов
    ui->serversTree->setSortingEnabled(true);
}

bool launcher::updateServerInformation(int serverIndex, int itemIndex) {

    qInfo() << "launcher::updateServerInformation: Upd. Online server Infomation - " << favServers[serverIndex].serverName;

                                // Сообщение "TSource query"
    unsigned char message[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45,
                                0x6E, 0x67, 0x69, 0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00};
    int length = 25;            // Длина отправляемой строки
    int ping;                   // Переменная для хранения значения пинга сервера
    int rLen;                   // Длина строки ответа

    // Запуск инструмента получения информации о сервере
    BYTE* response = exchangeDataWithServer(favServers[serverIndex].serverIP.toLatin1().data(), favServers[serverIndex].serverPort.toInt()+1, 400, message, length, ping, rLen);


    // Парсим ответ сервера
    if(ping>0) {                // Если получен ответ от сервера

        int pos = 6;                                                                   // Позиция начала строки
        favServers[serverIndex].ping = QString::number(ping);                              // Прописываем пинг
        favServers[serverIndex].HostName = QString(GetNextPart(pos, response, rLen));      // Прописываем имя сервера
        favServers[serverIndex].MapName = QString(GetNextPart(pos, response, rLen));       // Прописываем название карты
        favServers[serverIndex].GameType = QString(GetNextPart(pos, response, rLen));      // Прописываем тип игры
        favServers[serverIndex].GameMode = QString(GetNextPart(pos, response, rLen));      // Прописываем мод игры
        GetNextPart(pos, response, rLen);                                              // Пустая строка
        GetNextPart(pos, response, rLen);                                              // Пустая строка
        QByteArray pInfo = GetNextPart(pos, response, rLen);                          // Получаем кол-во игроков
        if((int)pInfo[0] >= 0 && (int)pInfo[1] >= 0) {
            if(pInfo.length()>0) {
                favServers[serverIndex].NumPlayers=QString::number((int)pInfo[0]);
                favServers[serverIndex].MaxPlayers=QString::number((int)pInfo[1]);
            } else {
                favServers[serverIndex].NumPlayers="0";
                favServers[serverIndex].MaxPlayers=QString::number((int)GetNextPart(pos, response, rLen)[0]);
            }
        } else {
            favServers[serverIndex].NumPlayers="-";
            favServers[serverIndex].MaxPlayers="-";
        }
        GetNextPart(pos, response, rLen);                                              // Enviroment. dw?? Visibility & vac?
        favServers[serverIndex].RequiredVersion = GetNextPart(pos, response, rLen);
        // Получение информации из GameTags
        QString GameTags = GetNextPart(pos, response, rLen);
        if(!GameTags.contains(",s",Qt::CaseInsensitive))
             GameTags = GetNextPart(pos, response, rLen);
        favServers[serverIndex].State = QString(GameTags[GameTags.indexOf(",s", Qt::CaseInsensitive)+2]).toInt();
    } else {                    // Если не получен ответ от сервера
        favServers[serverIndex].noResponse();
    }

    // Устанавливаем обновленную онлайн информацию о серверах
    QIcon icon;
    if(!favServers.at(serverIndex).ping.isEmpty()) {     // Если пинг есть, то присваиваем онлайн информацию
        ui->serversTree->topLevelItem(itemIndex)->setText(0, favServers[serverIndex].HostName);
        ui->selectServer->setItemText(serverIndex+1, favServers[serverIndex].HostName);
        ui->serversTree->topLevelItem(itemIndex)->setText(4, favServers[serverIndex].NumPlayers + "/" + favServers[serverIndex].MaxPlayers);
        QString serverState;
        switch(favServers[serverIndex].State) {
            case 7: serverState = tr("Игра"); break;
            case 6: serverState = tr("Брифинг"); break;
            case 3: serverState = tr("Лобби"); break;
            case 1: serverState = tr("Создание"); break;
            default: serverState = " - ";
        }
        ui->serversTree->topLevelItem(itemIndex)->setText(5, serverState);
        ui->serversTree->topLevelItem(itemIndex)->setText(6, favServers[serverIndex].ping);
        icon.addFile(QStringLiteral(":/myresources/serverOn.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree->topLevelItem(itemIndex)->setIcon(6, icon);
        ui->selectServer->setItemIcon(serverIndex+1, icon);
    }
    else { // Если не пингуется, то присваиваем стандартную информацию
        ui->serversTree->topLevelItem(itemIndex)->setText(0, favServers[serverIndex].serverName);
        ui->selectServer->setItemText(serverIndex+1, favServers[serverIndex].serverName);
        ui->serversTree->topLevelItem(itemIndex)->setText(4, "-/-");
        ui->serversTree->topLevelItem(itemIndex)->setText(5, " - ");
        ui->serversTree->topLevelItem(itemIndex)->setText(6, " - ");
        icon.addFile(QStringLiteral(":/myresources/serverOff.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->serversTree->topLevelItem(itemIndex)->setIcon(6, icon);
        ui->selectServer->setItemIcon(serverIndex+1, icon);
    }

    return (ping>0)?true:false;
}

// Выделение подстрок в байтовом массиве
QByteArray launcher::GetNextPart(int &pos, BYTE* response, int rLen) {
    QByteArray part;
    for (; pos<rLen && response[pos] != 0x0; pos++) {
        part.append(response[pos]);
    }
    pos++;
    return part;
}

// Слот начала слежения за выбранным элементов сервера
void launcher::on_serversTree_monitoring_clicked() {

    if(timerCheckServerStatus->isActive()) {   // Если слежение уже запущено, то останавливаем слежение
        qInfo() << "launcher::on_serversTree_follow_clicked: Unfollow server - " << favServers[monitoringServerItem->data(0, Qt::UserRole).toUInt()].HostName;
        popupMessage(tr("Слежение за серверов"), tr("Слежение за сервером ") + favServers[monitoringServerItem->data(0, Qt::UserRole).toUInt()].serverName + tr(" остановлено"));
        timerCheckServerStatus->stop();
    } else {    // Если слежение не запущено, начинаем следить за выбранным сервером
        if(ui->serversTree->currentIndex().row() != -1) { // Проверяем, выбран ли элемент
            monitoringServerItem = ui->serversTree->currentItem();
            oldServerState = -1;
            popupMessage(tr("Слежение за серверов"), tr("Слежение за сервером ") + favServers[monitoringServerItem->data(0, Qt::UserRole).toUInt()].serverName + tr(" началось"));
            checkServerStatus();
            timerCheckServerStatus->start(settings.updateTime);

            qInfo() << "launcher::on_serversTree_follow_clicked: Follow server - " << favServers[monitoringServerItem->data(0, Qt::UserRole).toUInt()].HostName;
        }
    }
}

// Проверка изменений состояния сервера
void launcher::checkServerStatus() {
    int monitoringServerIndex = monitoringServerItem->data(0, Qt::UserRole).toUInt();
    qInfo() << "launcher::checkServerStatus: Check server status - " << favServers[monitoringServerIndex].serverName;

    // Обновляем информацию о сервере и проверяем изменение статуса
    if(updateServerInformation(monitoringServerIndex, ui->serversTree->indexOfTopLevelItem(monitoringServerItem))) {

        // Проверяем, изменен ли статус сервера
        int currentState = favServers[monitoringServerIndex].State;
        if(currentState == 7 || currentState == 6 || currentState == 3 || currentState == 1) {
            if(oldServerState != -1 && oldServerState != favServers[monitoringServerIndex].State) { // Если статус сервера изменен, сообщаем об этом пользователю
                QString serverState;
                switch(settings.state) {
                case 0:
                    switch(currentState) {
                        case 7: serverState = tr("Игра"); break;
                        case 6: serverState = tr("Брифинг"); break;
                        case 3: serverState = tr("Лобби"); break;
                        case 1: serverState = tr("Создание"); break;
                        default: return;
                    }
                    break;
                case 1: if(currentState == 7) serverState = tr("Игра"); else return; break;
                case 2: if(currentState == 6) serverState = tr("Брифинг"); else return; break;
                case 3: if(currentState == 3) serverState = tr("Лобби"); else return; break;
                case 4: if(currentState == 1) serverState = tr("Создание"); else return; break;
                default: return;
                }
                popupMessage(tr("Изменение статуса сервера"), tr("Сервер ") + favServers[monitoringServerIndex].serverName + tr("\nизменил статус на \"") + serverState + "\"");
            }
            // Передаем текущий статус сервера
            oldServerState = favServers[monitoringServerIndex].State;
        }
    }
}
