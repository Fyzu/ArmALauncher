#include "launcher.h"
#include "ui_launcher.h"

/*
 * Функции работы со списком репозиториев
 */
// Слот смены выбранного элемента списка репозиториев
void launcher::on_repoList_currentRowChanged(int currentRow) {

    qInfo() << "launcher::on_repoList_currentRowChanged: row -" << currentRow;

    if(ui->repositoryList->tabText(1) == tr("Не подключен") && currentRow != -1) {
        ui->repoConnect->setEnabled(true);
    } else {
        ui->repoConnect->setEnabled(false);
    }
    if(currentRow != -1) {          // Если выбран элемент
        ui->repoEdit->setEnabled(true);
        ui->repoDel->setEnabled(true);
    } else { // Если элемент не выбран
        ui->repoEdit->setEnabled(false);
        ui->repoDel->setEnabled(false);
    }
}

// Слот кнопки "добавить репозиторий"
void launcher::on_repoAdd_clicked() {

    qInfo() << "launcher::on_repoAdd_clicked: new repo add";

    emit repoEditStart(Repository(), -1, true);
}

// Слот кнопки "удалить репозиторий"
void launcher::on_repoDel_clicked() {

    // Получаем текущий выбранный элемент
    int currentRow = ui->repoList->currentRow();

    // Удаляем выбранный элемент
    if(currentRow != -1) {          // Если выбран элемент

        qInfo() << "launcher::on_repoDel_clicked: repository del -" << currentRow;

        //..удаление в списке репозиториев
        repositories.takeAt(currentRow);
        //..убераем выбор элемента
        ui->repoList->setCurrentRow(-1);

        updateInformationInWidget();
    }
}

// Слот кнопки "сохранить репозиторий"
void launcher::on_repoEdit_clicked() {

    // Получаем текущий выбранный элемент
    int currentRow = ui->repoList->currentRow();

    // Проверяем выбран ли элемент
    if(currentRow != -1) {          // Если выбран элемент

        qInfo() << "launcher::updaterCheckAddonsUI: repository edit -" << currentRow;

        //..вносим данные в память
        emit repoEditStart(repositories[currentRow], currentRow, false);
    }
}

// Репозиторий изменен\добавлен и обновляем информацию в списке
void launcher::repoEditFinish(Repository repo, int currentRow, bool newRepo) {

    // Выбор, добавлен новый репозиторий или изменен старый
    if(newRepo) {
        repositories.append(repo);
        currentRow = repositories.size();
    } else {
        repositories[currentRow] = repo;
    }

    // Обновляем информации в виджетах
    updateInformationInWidget();

    // Выделяем измененный виджет
    ui->repoList->setCurrentRow(currentRow);
}

// Подключение к выбранному репозиторию
void launcher::on_repoConnect_clicked() {

    // Получаем текущий выбранный элемент
    int currentRow = ui->repoList->currentRow();

    // Проверяем, выбран ли элемент и не пустой ли Url
    if(!updaterIsRunning && currentRow != -1 && !repositories[currentRow].url.isEmpty() && ui->repositoryList->tabText(1) == tr("Не подключен")) {

        // Проверяем, есть директории для поиска аддонов
        if(pathFolder.isEmpty()) {
            QMessageBox::warning(this,tr("Внимание!"), tr("Не найдены директории для поиска аддонов.\nДобавьте директории для поиска аддонов или укажите исполняемый файлы Arma 3."), QMessageBox::Ok);
            qWarning() << "launcher::on_repoConnect_clicked: connect fail - addons folders - empty";
            return;
        }

        qInfo() << "launcher::on_repoConnect_clicked: repository connecting";

        // Переходим во вкладку репозитория
        ui->repositoryList->setCurrentIndex(1);

        // Отключаем лишний функционал UI апдейтера
        ui->checkAddons->setEnabled     (false);
        ui->downloadUpdate->setEnabled  (false);
        ui->delOtherFiles->setEnabled   (false);
        ui->stopUpdater->setEnabled     (false);
        ui->repoDisconnect->setEnabled  (false);
        ui->repoConnect->setEnabled     (false);

        // Сбрасываем UI апдейтера на дефолт
        ui->progressBar_all->setMaximum(1);
        ui->progressBar_all->setValue(0);
        ui->progressBar_current->setMaximum(1);
        ui->progressBar_current->setValue(0);
        ui->progressBar_all_label->setText(tr("Всего: "));
        ui->progressBar_all_label2->setText("0/0");
        ui->progressBar_current_label->setText(tr("Текущий: "));
        ui->progressBar_current_label2->setText("0/0");
        ui->progressBar_current_label3->setText("");
        ui->addonsTree->clear();
        ui->filesTree->clear();

        // Сброс переменных
        otherFiles.clear();
        newFiles.clear();
        correctFiles.clear();
        notCorrectFiles.clear();

        // Иницилизируем updater
        updater->setRepository(repositories[currentRow]);
        updaterIsRunning = true;

        //..запускаем апдейтер
        emit showUpdater();
    }
}

// Отключаем репозиторий
void launcher::on_repoDisconnect_clicked() {

    qInfo() << "launcher::on_repoDisconnect_clicked: repo: " << ui->repositoryList->tabText(1);

    updaterFinished();
    emit repositoryDisconnect();

    updaterIsRunning = false;
    ui->repoConnect->setEnabled(true);
    ui->repositoryList->setCurrentIndex(0);

    // Очищаем переменные апдейтера
    modsL.clear();
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();
}

/*
 * СЛОТЫ ИНТЕРФЕЙСА
 */
// Заполнение UI после успешного запуска апдейтера
void launcher::updaterStarted(const Repository repository, const QList< QMap<QString, QString> > addonsList, const QStringList modsList, bool success, QString defaultAddonsPath) {

    // Проверяем на успешность запуска
    if (success) {         // Если запуск прошел успешно

        qInfo() << "launcher::updaterStarted: repo start - succ";

        // Устанавливаем папку для проверки аддонов
        if(!defaultAddonsPath.isEmpty())
            ui->addonsFolders->setCurrentText(defaultAddonsPath);

        // Оповещаем пользователя, что подключение к репозиторию прошло успешно
        ui->repositoryList->setTabText(1, repository.name);
        ui->str1->setText(tr("Подключение прошло успешно!"));
        ui->str2->setText(tr("Всего    файлов: ") + QString::number(addonsList.count()));
        ui->str3->setText(tr("Нужных файлов: -"));
        //..включаем доступный функционал
        ui->checkAddons->setEnabled     (true);
        ui->repoDisconnect->setEnabled  (true);
        ui->checksum->setEnabled        (true);
        ui->addonsFolders->setEnabled   (true);

        // Заполняем дерево аддонов
        //..очищаем дерево аддонов
        ui->addonsTree->clear();
        //..временные переменные
        QTreeWidgetItem *item;
        QStringList addonsFolders;
        bool userconfigExists = false;
        //..цикл поиска нужных элементов, для добавления их в список
        if(repository.type == 0) { // Если репозиторий Yoma Addon Sync 2009
            auto endMod = modsList.constEnd();
            for(auto itMod = modsList.constBegin(); itMod != endMod; ++itMod) {
                // Добавляем userconfig
                if((*itMod).contains("userconfig") && !(*itMod).contains("\\userconfig")
                   && !userconfigExists) { // Если строка содержит юзерконфиг и не является дочерней папкой и в список не добавлялась
                    userconfigExists = true;                // Сигнализируем, что Userconfig есть и мы его добавили
                    // Добавляем элемент и прописываем его параметры
                    addonsFolders.append("userconfig");
                    item = new QTreeWidgetItem(ui->addonsTree);
                    item->setBackground(0, QBrush(Qt::gray));
                    item->setText(0, "userconfig");
                    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                    item->setCheckState(0, Qt::Unchecked);    // Заранее делаем выбранным, ибо это важный раздел
                } else
                // Добавляем основной аддон
                if((itMod+1) != endMod && (*itMod) == (*(itMod+1))) { // Если это не последний элемент списка и текущий элемент равен следующему
                    // Добавляем элемент и прописываем его параметры
                    addonsFolders.append((*itMod));
                    item = new QTreeWidgetItem(ui->addonsTree);
                    item->setBackground(0, QBrush(Qt::gray));
                    item->setText(0, (*itMod));
                    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                    item->setCheckState(0, Qt::Checked);    // Заранее делаем выбранным, ибо это важный раздел
                    ++itMod;                                    // Перескакиваем элемент, т.к. элементы одинаковые
                } else
                // Добавляем не основной раздел
                if (itMod != modsList.constBegin() && (*itMod) != (*(itMod-1)) && !(*itMod).contains('\\')) { // Если это не первый элемент и предыдущий элемент не является таким же,
                                                                                            //  так же элемент не содержит '\'
                    // Добавляем элемент и прописываем его параметры
                    addonsFolders.append((*itMod));
                    item = new QTreeWidgetItem(ui->addonsTree);
                    item->setBackground(0, QBrush(Qt::gray));
                    item->setText(0, (*itMod));
                    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                    item->setCheckState(0, Qt::Unchecked);    // Делаем раздел не выбранным, т.к. этот раздел не относится к важным
                }
            }
        } else { // Если репозиторий типа - Arma3Sync
            auto endMod = modsList.constEnd();
            for(auto itMod = modsList.constBegin(); itMod != endMod; ++itMod) {
                // Добавляем элемент и прописываем его параметры
                addonsFolders.append((*itMod));
                item = new QTreeWidgetItem(ui->addonsTree);
                item->setBackground(0, QBrush(Qt::gray));
                item->setText(0, (*itMod));
                item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                if((*itMod).startsWith('@'))
                    item->setCheckState(0, Qt::Checked);
                else
                    item->setCheckState(0, Qt::Unchecked);
            }
        }

        // Сообщаем апдейтеру что запуск прошел успешно
        modsL = addonsFolders;
        emit updaterUIStarted(addonsFolders);

    } else { // Если запуск прошел не успешно
        qWarning() << "launcher::updaterStarted: repo start - fail";
        on_repoDisconnect_clicked();
        QMessageBox::warning(this,tr("Внимание!"), tr("Неудалось подключится к репозиторию,\nв процессе возникли ошибки."), QMessageBox::Ok);
    }

}

// Отановка апдейтера - слот UI
void launcher::stopUpdaterUI() {
    // Сигнализируем что бы апдейтер остановился
    if(stopUpdaterInProcess) {
        qInfo() << "launcher::stopUpdaterUI: stop";
        stopUpdaterInProcess = false;
        emit stopUpdater();
    }
}

// Заполнение UI после завершения работы апдейтера (отключение или ошибка в подключении)
void launcher::updaterFinished() {

    qInfo() << "launcher::updaterFinished: finish";

    // Оповещаем пользователя, что апдейтер завершился
    ui->repositoryList->setTabText(1, tr("Не подключен"));
    ui->str1->setText(tr(" - Репозиторий не подключен -"));
    ui->str2->setText(tr("Для подключения, зайдийте во"));
    ui->str3->setText(tr("вкладку репозитории."));
    ui->progressBar_all->setMaximum(1);
    ui->progressBar_all->setValue(0);
    ui->progressBar_current->setMaximum(1);
    ui->progressBar_current->setValue(0);
    ui->progressBar_all_label->setText(tr("Всего: "));
    ui->progressBar_all_label2->setText("0/0");
    ui->progressBar_current_label->setText(tr("Текущий: "));
    ui->progressBar_current_label2->setText("0/0");
    ui->progressBar_current_label3->setText("");

    // Отключаем функционал UI апдейтера
    ui->checkAddons->setEnabled     (false);
    ui->downloadUpdate->setEnabled  (false);
    ui->delOtherFiles->setEnabled   (false);
    ui->stopUpdater->setEnabled     (false);
    ui->addonsFolders->setEnabled   (false);
    ui->repoDisconnect->setEnabled  (false);
    ui->checksum->setEnabled        (false);
    ui->addonsFolders->setEnabled   (false);

    // Очищаем ненужные поля
    ui->addonsTree->clear();
    ui->filesTree->clear();

    disconnect(ui->addonsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this,           SLOT(addonsTreeCheck(QTreeWidgetItem*)));
}

// Слот при нажатии на кнопку проверить аддоны
void launcher::updaterCheckAddonsUI() {

    // Подготавливаем UI и сигнализируем апдейтеру о проверке
    if(!checkAddonsIsRunning) {

        qInfo() << "launcher::updaterCheckAddonsUI: checkaddons button clicked";

        // Оповещаем пользователя, что начата проверка файлов
        ui->str1->setText("Проверка аддонов начата");
        ui->progressBar_all_label->setText(tr("Всего: Проверка файлов начата"));
        ui->progressBar_all_label2->setText("0/1");
        ui->progressBar_current->setMaximum(0);
        ui->progressBar_current->setValue(0);
        ui->progressBar_current_label2->clear();
        ui->progressBar_current_label3->clear();

        // Отключаем функционал который невозможно вызвать
        ui->checkAddons->setEnabled   (false);
        ui->downloadUpdate->setEnabled(false);
        ui->delOtherFiles->setEnabled (false);
        ui->addonsFolders->setEnabled (false);

        // Активируем кнопку остановки процесса
        ui->stopUpdater->setEnabled(true);
        stopUpdaterInProcess = true;
        ui->repoDisconnect->setEnabled(true);

        // Сообщаем апдейтеру, что UI готов
        checkAddonsIsRunning = true;
        emit updaterCheckAddons(ui->addonsFolders->currentText());
    }
}

// Слот UI - когда дочерний поток завершил проверку файлов
void launcher::checkAddonsFinishedUI(int type, const QList< QMap<QString, QString> > otherF,   const QList< QMap<QString, QString> > newF,
                                               const QList< QMap<QString, QString> > correctF, const QList< QMap<QString, QString> > notCorrectF) {

    qInfo() << "launcher::checkAddonsFinishedUI: finish";

    // Получаем список файлов для UI
    otherFiles = otherF;
    newFiles = newF;
    correctFiles = correctF;
    notCorrectFiles = notCorrectF;
    QStringList modsList = modsL;

    // Включаем\отключаем нужный функционал
    //..отключаем
    ui->stopUpdater->setEnabled(false);
    //..включаем
    ui->downloadUpdate->setEnabled(true);
    if(otherF.size() == 0)
        ui->delOtherFiles->setEnabled(false);
    else
        ui->delOtherFiles->setEnabled(true);

    // Заполняем строки информацией
    ui->str1->setText(tr("Проверка прошла успешно!"));
    ui->str3->setText(tr("Нужных файлов: ") + QString::number(newFiles.size()+notCorrectFiles.size()));
    ui->progressBar_all_label->setText(tr("Всего: Проверка файлов прошла успешно"));
    ui->progressBar_all->setMaximum(1);
    ui->progressBar_all->setValue(1);
    ui->progressBar_current->setMaximum(1);
    ui->progressBar_current->setValue(1);
    ui->progressBar_current_label2->clear();
    ui->progressBar_current_label3->clear();

    // Заполнение информацией дерево файлов
    //..очищаем дерево
    ui->filesTree->clear();
    //..заполняем правильными файлами
    auto correctEnd = correctFiles.constEnd();
    for(auto itFile = correctFiles.constBegin();  itFile != correctEnd; ++itFile) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, (*itFile)["Path"]+"\\"+(*itFile)["Pbo"]);
        if(type == 0) {
            item->setText(1, (*itFile)["Md5"]);
            item->setText(2, (*itFile)["Md5"]);
        } else {
            item->setText(1, (*itFile)["Sha1"]);
            item->setText(2, (*itFile)["Sha1"]);
        }
        item->setBackground(0, QBrush(Qt::green));
        item->setBackground(1, QBrush(Qt::green));
        item->setBackground(2, QBrush(Qt::green));
    }
    //..заполняем не правильными файлами
    auto notcorrectEnd = notCorrectFiles.constEnd();
    for(auto itFile = notCorrectFiles.constBegin();  itFile != notcorrectEnd; ++itFile) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, (*itFile)["Path"]+"\\"+(*itFile)["Pbo"]);
        if(type == 0) {
            item->setText(1, (*itFile)["Md5"]);
            item->setText(2, (*itFile)["Md5local"]);
        } else {
            item->setText(1, (*itFile)["Sha1"]);
            item->setText(2, (*itFile)["Sha1local"]);
        }
        item->setBackground(0, QBrush(Qt::yellow));
        item->setBackground(1, QBrush(Qt::yellow));
        item->setBackground(2, QBrush(Qt::yellow));
        // Создаем список корректных аддонов
        for(auto itMod = modsList.begin(); itMod != modsList.end(); ++itMod) {
            if((*itFile)["Path"].contains((*itMod))
              && !(*itFile)["Path"].contains('\\' + (*itMod) +'\\') && !(*itFile)["Path"].contains('\\' + (*itMod))) {
                modsList.erase(itMod);
                break;
            }
        }
    }
    //..заполняем новыми файлами
    auto newEnd = newFiles.constEnd();
    for(auto itFile = newFiles.constBegin();  itFile != newEnd; ++itFile) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, (*itFile)["Path"]+"\\"+(*itFile)["Pbo"]);
        item->setText(1, (*itFile)["Md5"]);
        item->setText(2, "NEW");
        item->setBackground(0, QBrush(Qt::cyan));
        item->setBackground(1, QBrush(Qt::cyan));
        item->setBackground(2, QBrush(Qt::cyan));
        // Создаем список корректных аддонов
        for(auto itMod = modsList.begin(); itMod != modsList.end(); ++itMod) {
            if((*itFile)["Path"].contains((*itMod))
              && !(*itFile)["Path"].contains('\\' + (*itMod) +'\\') && !(*itFile)["Path"].contains('\\' + (*itMod))) {
                modsList.erase(itMod);
                break;
            }
        }
    }
    //..заполняем лишними файлами
    auto otherEnd = otherFiles.constEnd();
    for(auto itFile = otherFiles.constBegin();  itFile != otherEnd; ++itFile) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, (*itFile)["Path"]+"\\"+(*itFile)["Pbo"]);
        item->setText(1, "DELETE");
        item->setText(2, " - ");
        item->setBackground(0, QBrush(Qt::red));
        item->setBackground(1, QBrush(Qt::red));
        item->setBackground(2, QBrush(Qt::red));
    }

    // Выявление аддонов, которым необходимо обновление
    for(QTreeWidgetItemIterator it(ui->addonsTree); (*it);++it) {
        if(modsList.contains((*it)->text(0), Qt::CaseInsensitive)) {
            (*it)->setCheckState(0, Qt::Unchecked);
            (*it)->setBackground(0, QBrush(Qt::green));
            (*it)->setIcon(0, QIcon(":/myresources/IMG/checked21.png"));
        } else {
            addonsTreeCheck((*it));
            (*it)->setBackground(0, QBrush(Qt::yellow));
            (*it)->setIcon(0, QIcon(":/myresources/IMG/warning37.png"));
        }
    }

    // Если меняется checkState какого-либо аддона, то меняется и цепочка файлов
    connect(ui->addonsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this,           SLOT(addonsTreeCheck(QTreeWidgetItem*)));

    checkAddonsIsRunning = false;
}

// Обновление прогресса проверки аддонов в UI
void launcher::checkAddonsProgressUI(int index, const QList< QMap<QString, QString> > existsFiles) {

    ui->progressBar_all_label2->setText(QString::number(index+1) + "/" + QString::number(existsFiles.size()));
    ui->progressBar_all->setMaximum(existsFiles.size());
    ui->progressBar_all->setValue(index+1);
    ui->progressBar_current_label->setText(tr("Текущий: Проверяем ") + existsFiles[index]["Pbo"]);
}

// Начата загрузка обновления - оповещаем UI
void launcher::downloadUpdateStartUI() {

    // Проверяем, запущена ли арма, если да, сообщить об этом
    if(getHandle("Arma 3", false)) {
        QMessageBox::warning(this,tr("Внимание!"), tr("Невозможно начать обновление аддонов, т.к. Arma 3 запущена.\nЗакройте Arma 3, для безопасного обновления аддонов."), QMessageBox::Ok);
        qWarning() << "launcher::on_repoConnect_clicked: download fail - arma3 is running";
        return;
    }

    // Получаем список ID файлов которые выбраны для скачивания
    QList<int> fileID;
    int count = correctFiles.size() + notCorrectFiles.size() + newFiles.size();
    for(int i = correctFiles.size(); i<count;i++) {
        QTreeWidgetItem *item = ui->filesTree->topLevelItem(i);
        if(item->checkState(0) == Qt::Checked) {
            fileID.append(i-correctFiles.size());
        }
    }

    // Проверяем, есть ли файлы на скачивание
    if(fileID.size() > 0) {

        qInfo() << "launcher::downloadUpdateStartUI: download -" << fileID.size() << "files";

        // Отключаем/включаем функционал
        ui->stopUpdater->setEnabled(true);
        stopUpdaterInProcess = true;
        ui->repoDisconnect->setEnabled(false);
        ui->downloadUpdate->setEnabled(false);
        ui->delOtherFiles->setEnabled(false);

        // Оповещаем UI, что загрузка начата
        ui->str1->setText("Загрузка аддонов начата");
        ui->progressBar_all_label->setText(tr("Всего: Скачиваем аддоны"));
        ui->progressBar_all_label2->setText("0/1");
        ui->progressBar_all->setMaximum(fileID.size());
        ui->progressBar_all->setValue(0);

        downloadAddonsIsRunning = true;
        totalSize = 0;

        emit downloadUpdateStart(fileID);
    } else {
        qWarning() << "launcher::downloadUpdateStartUI: not found download files";
        QMessageBox::warning(this,tr("Внимание!"), tr("Нет файлов для загрузки с репозитория.\nВы ничего не выбрали для загрузки или все файлы скачены."), QMessageBox::Ok);
    }
}

// Начата загрузка нового файла - оповещаем UI
void launcher::downloadAddonStartedUI(const QList< QMap<QString, QString> > downloadFiles, int index) {

    qInfo() << "launcher::downloadAddonStartedUI: file -" << downloadFiles[index];

    // Считаем сколько всего нужно скачать и сколько скачали
    qlonglong currentSize = 0;
    //..всего скачать
    if(totalSize == 0)
        for(int i = 0; i < downloadFiles.size();i++) {
            totalSize += downloadFiles[i]["Size"].toLongLong();
        }
    //..всего скачали
    for(int i = 1; i<= index;i++) {
        currentSize += downloadFiles[i-1]["Size"].toLongLong();
    }

    // Регистрируем прогресс в прогресс баре
    ui->progressBar_all->setValue(index+1);

    // Описываем сколько осталось до конца загрузки из скольки, приводя к разумным типам исчесления данных
    // Если байты не больше..
    //..1 КБ
    if(totalSize < 1024)
        ui->progressBar_all_label2->setText(QString::number(currentSize)  + "/"
                                          + QString::number(totalSize) + tr(" байт"));
    //..1 МБ
    else if (totalSize < 1048576)
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1024))  + "/"
                                          + QString::number((int)(totalSize/1024)) + tr(" КБ"));
    //..ГБ
    else if (totalSize < 4294967296)
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1048576))  + "/"
                                          + QString::number((int)(totalSize/1048576)) + tr(" МБ"));
    // Если больше или равны 4 ГБ
    else
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1048576))  + tr(" МБ/")
                                          + QString::number((int)(totalSize/4294967296)) + tr(" ГБ"));
}

// Завершена загрузка файлов - оповещаем UI
void launcher::downloadAddonsFinishUI(bool success) {

    qInfo() << "launcher::downloadAddonsFinishUI: addons download succ -" << success;

    // Заполняем UI в соответсвии с успешностью загрузки
    ui->progressBar_all->setMaximum(1);
    ui->progressBar_all->setValue(1);
    if(!success) {
        ui->str1->setText("Загрузка аддонов остановлена");
        ui->progressBar_all_label->setText(tr("Всего: Загрузка аддонов не закончена"));
    } else {
        ui->str1->setText("Загрузка аддонов успешна");
        ui->progressBar_all_label->setText(tr("Всего: Загрузка аддонов прошла успешно"));
    }

    // Отключаем/включаем ненужный функционал
    ui->downloadUpdate->setEnabled(false);
    ui->stopUpdater->setEnabled(false);
    ui->repoDisconnect->setEnabled(true);
    ui->checkAddons->setEnabled(true);
    //..если есть что удалять, то включаем удаление лишних файлов
    if(otherFiles.size() == 0)
        ui->delOtherFiles->setEnabled(false);
    else
        ui->delOtherFiles->setEnabled(true);

    downloadAddonsIsRunning = false;

    popupMessage("Обновление аддонов завершено", "Все выбранные аддоны успешно обновлены." + (otherFiles.size() > 0)? "\nНо остались лишние файлы, которых нет в данном репозитории.":"\nЛишних файлов не обнаружено.");
}

// Слот завершения удаления лишних файлов
void launcher::deleteOtherFilesFinish() {

    qInfo() << "launcher::deleteOtherFilesFinish: finish";

    ui->delOtherFiles->setEnabled(false);
    ui->checkAddons->setEnabled(true);
}

// Меняем checkState цепочки файлов
void launcher::addonsTreeCheck(QTreeWidgetItem *item) {

    qInfo() << "launcher::addonsTreeCheck: addonsTree - item changed " << item->text(0);

    // Получаем имя аддона, checkState которого изменился
    QString addon = item->text(0);

    // Перебираем списрк файлов и высталяем checkState
    for(int i = 0; i<ui->filesTree->topLevelItemCount(); i++) {
        //Получаем указатель на файл
        QTreeWidgetItem *file = ui->filesTree->topLevelItem(i);
        // Выставляем checkState файлам, которые буду загружаться, а это (yellow & cyan)
        if(file->text(0).contains(addon+'\\') && !file->text(0).contains('\\' + addon) &&
          (file->backgroundColor(0) == Qt::yellow || file->backgroundColor(0) == Qt::cyan)) {
            //..выставляем checkState аддона
            file->setCheckState(0, item->checkState(0));
        }
    }
}

// Меняем отображение информации в filesTree
void launcher::on_checksum_stateChanged(int stateCheck) {

    // Если checkState = Uncheked, то отображаем лишь файлы
    if(stateCheck == Qt::Unchecked) {

        qInfo() << "launcher::on_checksum_stateChanged: checksum hide";

        ui->filesTree->setColumnCount(1);
    } else {// Иначе, то отображаем и МД5

        qInfo() << "launcher::on_checksum_stateChanged: checksum show";

        ui->filesTree->setColumnCount(3);
        //..размер секций filesTree
        ui->filesTree->header()->resizeSection(0, 200);
        ui->filesTree->header()->resizeSection(1, 90);
        ui->filesTree->header()->resizeSection(2, 80);
    }
}

/*
 * Cлоты загрузки файлов
 */
// Обновление данных в UI о старте загрузки файла
void launcher::downloadFile_UI(QString fileName) {

    qInfo() << "launcher::downloadFile_UI: file download -" << fileName;

    // Оповещаем UI какой файл мы скачиваем
    ui->progressBar_current_label->setText(tr("Текущий: Скачиваем ") + fileName);

    // Включаем кнопку остановк загрузки
    ui->stopUpdater->setEnabled(true);
    stopUpdaterInProcess = true;
}

// Обновление данных в UI о прогрессе загрузки файла
void launcher::updateDownloadProgress_UI(qint64 bytesRead, qint64 totalBytes, QString speed) {

    // Обновляем индикатор скорости загрузки
    if(!speed.isEmpty())
        ui->progressBar_current_label3->setText(speed);

    // Регистрируем прогресс на прогресс баре
    ui->progressBar_current->setValue(bytesRead);
    ui->progressBar_current->setMaximum(totalBytes);

    // Описываем сколько осталось до конца загрузки из скольки, приводя к разумным типам исчесления данных
    // Если байты не больше..
    //..1 КБ
    if(totalBytes < 1024)
        ui->progressBar_current_label2->setText(QString::number(bytesRead)  + "/"
                                              + QString::number(totalBytes) + tr(" байт"));
    //..1 МБ
    else if (totalBytes < 1048576)
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1024))  + "/"
                                              + QString::number((int)(totalBytes/1024)) + tr(" КБ"));
    //..ГБ
    else if (totalBytes < 1073741824)
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1048576))  + "/"
                                              + QString::number((int)(totalBytes/1048576)) + tr(" МБ"));
    // Если больше или равны 1 ГБ
    else
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1048576))  + tr(" МБ/")
                                              + QString::number((int)(totalBytes/1073741824)) + tr(" ГБ"));
}

// Обновлении данных в UI о завершении загрузки файла
void launcher::downloadFinished(bool success) {

    // Отключаем кнопку остановки
    ui->stopUpdater->setEnabled(false);

    // Проверка успешной загрузки файла
    if(success) {             // Если файл скачен успешно

        qInfo() << "launcher::downloadFinished: success";

        ui->progressBar_current_label->setText(ui->progressBar_current_label->text().replace(tr("Скачиваем"), tr("Скачен")));
    } else {                    // Если нет

        qInfo() << "launcher::downloadFinished: fail";

        ui->progressBar_current_label->setText(tr("Текущий: Скачивание остановлено"));
    }
}

/*
 * Слоты unzip'а
 */
// Обновлении данных в UI о начале распаковки
void launcher::unzipStart(QString fileName) {

    qInfo() << "launcher::unzipStart: start";

    // Оповещаем пользователя, что началась распаковка
    ui->progressBar_current_label->setText(tr("Текущий: Распаковываем ") + fileName);
    ui->progressBar_current->setValue(0);
    ui->progressBar_current->setMaximum(1);
}

// Обновлении данных в UI о завершении распаковки
void launcher::unzipFinished(QString fileName) {

    qInfo() << "launcher::unzipFinished: finish";

    // Оповещаем пользователя, что распаковка закончилась
    ui->progressBar_current_label->setText(tr("Текущий: Распаковали ") + fileName);
    ui->progressBar_current->setValue(1);
    ui->progressBar_current->setMaximum(1);
}

// Слот ошибок в апдейтере
void launcher::errorUI(int type, QString msg) {
    if (type == 0) {
        QMessageBox::warning(this,tr("Внимание!"), tr("Во время загрузки файла возникли ошибки.\nОшибка: ") + msg, QMessageBox::Ok);
    } else if(type == 1) {
        QMessageBox::warning(this,tr("Внимание!"), tr("Во время получения информации от репозитория возникли ошибки.\nОшибка: ") + msg, QMessageBox::Ok);
    } else if(type == 2) {
        QMessageBox::warning(this,tr("Внимание!"), tr("Во время распаковки архива возникли ошибки.\n7z output: ") + msg, QMessageBox::Ok);
    }
}
