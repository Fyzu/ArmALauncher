#include "launcher.h"
#include "ui_launcher.h"

/*
 * Функции работы со списком репозиториев
 */
// Слот смены выбранного элемента списка репозиториев
void launcher::on_repoList_currentRowChanged(int currentRow) {

    if(ui->repositoryList->tabText(1) == "Не подключен" && currentRow != -1) {
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

    qDebug() << "Debug-updateAddons_UI: repository Add";
    emit repoEditStart(Repository(), -1, true);
}

// Слот кнопки "удалить репозиторий"
void launcher::on_repoDel_clicked() {

    // Получаем текущий выбранный элемент
    int currentRow = ui->repoList->currentRow();

    // Удаляем выбранный элемент
    if(currentRow != -1) {          // Если выбран элемент

        qDebug() << "Debug-updateAddons_UI: repository Del";

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

        qDebug() << "Debug-updateAddons_UI: repository Edit";

        //..вносим данные в память
        emit repoEditStart(repositories[currentRow], currentRow, false);
    }
}

void launcher::repoEditFinish(Repository repo, int currentRow, bool newRepo) {

    if(newRepo) {
        repositories.append(repo);
        currentRow = repositories.size();
    } else {
        repositories[currentRow] = repo;
    }

    updateInformationInWidget();

    ui->repoList->setCurrentRow(currentRow);
}

// Подключение к выбранному репозиторию
void launcher::on_repoConnect_clicked() {

    // Получаем текущий выбранный элемент
    int currentRow = ui->repoList->currentRow();

    // Проверяем, выбран ли элемент и не пустой ли Url
    if(currentRow != -1 && !repositories[currentRow].url.isEmpty() && ui->repositoryList->tabText(1) == "Не подключен") {

        // Проверяем, есть директории для поиска аддонов
        if(ui->addonsFolders->count() == 0) {
            QMessageBox::warning(this,"Внимание!", "Не найдены директории для поиска аддонов.\nДобавьте директории для поиска аддонов или укажите исполняемый файлы Arma 3.", QMessageBox::Ok);
            return;
        }

        qDebug() << "Debug-updateAddons_UI: repository Connect";

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
        ui->progressBar_all_label->setText("Всего: ");
        ui->progressBar_all_label2->setText("0/0");
        ui->progressBar_current_label->setText("Текущий: ");
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

        //..запускаем апдейтер
        emit showUpdater();

        qDebug() << "Debug-updateAddons_UI: Updater thread - start";
    }
}

// Отключаем репозиторий
void launcher::on_repoDisconnect_clicked() {

    qDebug() << "Debug-updateAddons_UI: repository Disconnect";

    updaterFinished();
    emit repositoryDisconnect();

    ui->repoConnect->setEnabled(true);
    ui->repositoryList->setCurrentIndex(0);
}

/*
 * СЛОТЫ ИНТЕРФЕЙСА
 */
// Заполнение UI после успешного запуска апдейтера
void launcher::updaterStarted(const Repository repository, const QList< QMap<QString, QString> > addonsList, const QStringList modsList) {

    qDebug() << "Debug-updateAddons_UI: updater Started UI - start";

    modsL = modsList;

    // Оповещаем пользователя, что подключение к репозиторию прошло успешно
    ui->repositoryList->setTabText(1, repository.name);
    ui->str1->setText("Подключение прошло успешно!");
    ui->str2->setText("Всего    файлов: " + QString::number(addonsList.count()));
    ui->str3->setText("Нужных файлов: -");
    //..включаем доступный функционал
    ui->checkAddons->setEnabled     (true);
    ui->repoDisconnect->setEnabled  (true);
    ui->md5check->setEnabled        (true);
    ui->addonsFolders->setEnabled   (true);

    // Заполняем дерево аддонов
    //..очищаем дерево аддонов
    ui->addonsTree->clear();
    //..временные переменные
    QTreeWidgetItem *item;
    QStringList addonsFolders;
    bool userconfigExists = false;
    //..цикл поиска нужных элементов, для добавления их в список
    if(repository.type == 0)
        for(int i = 0; i<modsList.count();i++) {
            // Добавляем userconfig
            if(modsList[i].contains("userconfig") && !modsList[i].contains("\\userconfig")
               && !userconfigExists) { // Если строка содержит юзерконфиг и не является дочерней папкой и в список не добавлялась
                userconfigExists = true;                // Сигнализируем, что Userconfig есть и мы его добавили
                // Добавляем элемент и прописываем его параметры
                addonsFolders.append("userconfig");
                item = new QTreeWidgetItem(ui->addonsTree);
                item->setText(0, "userconfig");
                item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                item->setCheckState(0, Qt::Unchecked);    // Заранее делаем выбранным, ибо это важный раздел
            } else
            // Добавляем основной аддон
            if(i+1 < modsList.count() && modsList[i] == modsList[i+1]) { // Если это не последний элемент списка и текущий элемент равен следующему
                // Добавляем элемент и прописываем его параметры
                addonsFolders.append(modsList[i]);
                item = new QTreeWidgetItem(ui->addonsTree);
                item->setText(0, modsList[i]);
                item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                item->setCheckState(0, Qt::Checked);    // Заранее делаем выбранным, ибо это важный раздел
                i++;                                    // Перескакиваем элемент, т.к. элементы одинаковые
            } else
            // Добавляем не основной раздел
            if (i != 0 && modsList[i] != modsList[i-1] && !modsList[i].contains('\\')) { // Если это не первый элемент и предыдущий элемент не является таким же,
                                                                                        //  так же элемент не содержит '\'
                // Добавляем элемент и прописываем его параметры
                addonsFolders.append(modsList[i]);
                item = new QTreeWidgetItem(ui->addonsTree);
                item->setText(0, modsList[i]);
                item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                item->setCheckState(0, Qt::Unchecked);    // Делаем раздел не выбранным, т.к. этот раздел не относится к важным
            }
        }
    else
        for(int i = 0; i<modsList.count();i++) {
            // Добавляем элемент и прописываем его параметры
            addonsFolders.append(modsList[i]);
            item = new QTreeWidgetItem(ui->addonsTree);
            item->setText(0, modsList[i]);
            item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
            if(modsList[i].startsWith('@'))
                item->setCheckState(0, Qt::Checked);
            else
                item->setCheckState(0, Qt::Unchecked);
        }

    emit updaterUIStarted(addonsFolders);
    qDebug() << "Debug-updateAddons_UI: updater Started UI - succ";

}

void launcher::stopUpdaterUI() {
    if(stopUpdaterInProcess) {
        stopUpdaterInProcess = false;
        emit stopUpdater();
    }
}

// Заполнение UI после завершения работы апдейтера (отключение или ошибка в подключении)
void launcher::updaterFinished() {

    qDebug() << "Debug-updateAddons_UI: updater Finished UI - start";

    // Оповещаем пользователя, что апдейтер завершился
    ui->repositoryList->setTabText(1, "Не подключен");
    ui->str1->setText(" - Репозиторий не подключен -");
    ui->str2->setText("Для подключения, зайдийте во");
    ui->str3->setText("вкладку репозитории.");
    ui->progressBar_all->setMaximum(1);
    ui->progressBar_all->setValue(0);
    ui->progressBar_current->setMaximum(1);
    ui->progressBar_current->setValue(0);
    ui->progressBar_all_label->setText("Всего: ");
    ui->progressBar_all_label2->setText("0/0");
    ui->progressBar_current_label->setText("Текущий: ");
    ui->progressBar_current_label2->setText("0/0");
    ui->progressBar_current_label3->setText("");

    // Отключаем функционал UI апдейтера
    ui->checkAddons->setEnabled     (false);
    ui->downloadUpdate->setEnabled  (false);
    ui->delOtherFiles->setEnabled   (false);
    ui->stopUpdater->setEnabled     (false);
    ui->addonsFolders->setEnabled   (false);
    ui->repoDisconnect->setEnabled  (false);
    ui->md5check->setEnabled        (false);
    ui->addonsFolders->setEnabled   (false);

    // Очищаем ненужные поля
    ui->addonsTree->clear();
    ui->filesTree->clear();
    ui->progressBar_all->setValue(0);
    ui->progressBar_current->setValue(0);

    // Отвязываем слоты\сигналы
    /*disconnect(ui->checkAddons,SIGNAL(clicked()),this,SLOT(updaterCheckAddonsUI()));
    disconnect(this,SIGNAL(updaterCheckAddons(QString)),updater,SLOT(checkAddons(QString)));
    disconnect(updater,SIGNAL(checkAddonsFinished(int,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>)),this,SLOT(checkAddonsFinishedUI(int,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>,QList<QMap<QString,QString>>)));
    disconnect(this,SIGNAL(downloadUpdateStart(QList<int>)),updater,SLOT(downloadUpdate(QList<int>)));
    disconnect(updater,SIGNAL(downloadAddonStarted(QList<QMap<QString,QString>>,int)),this,SLOT(downloadAddonStartedUI(QList<QMap<QString,QString>>,int)));
    disconnect(updater,SIGNAL(downloadAddonsFinished(bool)),this,SLOT(downloadAddonsFinishUI(bool)));
    disconnect(ui->delOtherFiles,SIGNAL(clicked()),updater,SLOT(delOtherFiles()));
    disconnect(ui->stopUpdater,SIGNAL(clicked()),this,SLOT(stopUpdaterUI()));
    disconnect(this,SIGNAL(stopUpdater()),updater,SLOT(stopUpdater()));
    disconnect(updater,SIGNAL(updateDownloadProgress_UI(qint64,qint64,QString)),this,SLOT(updateDownloadProgress_UI(qint64,qint64,QString)));
    disconnect(updater,SIGNAL(downloadFile_UI(QString)),this,SLOT(downloadFile_UI(QString)));
    disconnect(updater,SIGNAL(downloadFinished(bool)),this,SLOT(downloadFinished(bool)));
    disconnect(updater,SIGNAL(unzipStart(QString)),this,SLOT(unzipStart(QString)));
    disconnect(updater,SIGNAL(unzipFinished(QString)),this,SLOT(unzipFinished(QString)));
    disconnect(updater,SIGNAL(finished()),this,SLOT(updaterFinished()));*/

    qDebug() << "Debug-updateAddons_UI: updater Finished UI - succ";
}

// Слот при нажатии на кнопку проверить аддоны
void launcher::updaterCheckAddonsUI() {

    qDebug() << "Debug-updateAddons_UI: pushButton clicked - updaterCheckAddonsUI - start";

    // Оповещаем пользователя, что начата проверка файлов
    ui->str1->setText("Проверка аддонов начата");
    ui->progressBar_all_label->setText("Всего: Проверка файлов начата");
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
    emit updaterCheckAddons(ui->addonsFolders->currentText());

    qDebug() << "Debug-updateAddons_UI: updaterCheckAddonsUI - succ";
}

// Слот UI - когда дочерний поток завершил проверку файлов
void launcher::checkAddonsFinishedUI(int type, const QList< QMap<QString, QString> > otherF, const QList< QMap<QString, QString> > newF,
                                     const QList< QMap<QString, QString> > correctF, const QList< QMap<QString, QString> > notCorrectF) {

    qDebug() << "Debug-updateAddons_UI: check Addons Finished UI - start";

    // Получаем список файлов для UI
    otherFiles = otherF;
    newFiles = newF;
    correctFiles = correctF;
    notCorrectFiles = notCorrectF;

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
    ui->str1->setText("Проверка прошла успешно!");
    ui->str3->setText("Нужных файлов: " + QString::number(newFiles.size()+notCorrectFiles.size()));
    ui->progressBar_all_label->setText("Всего: Проверка файлов прошла успешно");
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
    for(int index = 0; index < correctFiles.size();index++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, correctFiles[index]["Path"]+"\\"+correctFiles[index]["Pbo"]);
        if(type == 0) {
            item->setText(1, correctFiles[index]["Md5"]);
            item->setText(2, correctFiles[index]["Md5"]);
        } else {
            item->setText(1, correctFiles[index]["Sha1"]);
            item->setText(2, correctFiles[index]["Sha1"]);
        }
        item->setBackground(0, QBrush(Qt::green));
        item->setBackground(1, QBrush(Qt::green));
        item->setBackground(2, QBrush(Qt::green));
    }
    //..заполняем не правильными файлами
    for(int index = 0; index < notCorrectFiles.size();index++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, notCorrectFiles[index]["Path"]+"\\"+notCorrectFiles[index]["Pbo"]);
        if(type == 0) {
            item->setText(1, notCorrectFiles[index]["Md5"]);
            item->setText(2, notCorrectFiles[index]["Md5local"]);
        } else {
            item->setText(1, notCorrectFiles[index]["Sha1"]);
            item->setText(2, notCorrectFiles[index]["Sha1local"]);
        }
        item->setBackground(0, QBrush(Qt::yellow));
        item->setBackground(1, QBrush(Qt::yellow));
        item->setBackground(2, QBrush(Qt::yellow));
    }
    //..заполняем новыми файлами
    for(int index = 0; index < newFiles.size();index++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, newFiles[index]["Path"]+"\\"+newFiles[index]["Pbo"]);
        item->setText(1, newFiles[index]["Md5"]);
        item->setText(2, "NEW");
        item->setBackground(0, QBrush(Qt::cyan));
        item->setBackground(1, QBrush(Qt::cyan));
        item->setBackground(2, QBrush(Qt::cyan));
    }
    //..заполняем лишними файлами
    for(int index = 0; index < otherFiles.size();index++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->filesTree);
        item->setText(0, otherFiles[index]["Path"]+"\\"+otherFiles[index]["Pbo"]);
        item->setText(1, "DELETE");
        item->setText(2, " - ");
        item->setBackground(0, QBrush(Qt::red));
        item->setBackground(1, QBrush(Qt::red));
        item->setBackground(2, QBrush(Qt::red));
    }

    for(int i = 0; i < ui->addonsTree->topLevelItemCount(); i++)
        addonsTreeCheck(ui->addonsTree->topLevelItem(i));

    // Если меняется checkState какого-либо аддона, то меняется и цепочка файлов
    connect(ui->addonsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this,           SLOT(addonsTreeCheck(QTreeWidgetItem*)));

    qDebug() << "Debug-updateAddons_UI: check Addons Finished UI - succ";
}

// Обновление прогресса проверки аддонов в UI
void launcher::checkAddonsProgressUI(int index, const QList< QMap<QString, QString> > existsFiles) {

    ui->progressBar_all_label2->setText(QString::number(index+1) + "/" + QString::number(existsFiles.size()));
    ui->progressBar_all->setMaximum(existsFiles.size());
    ui->progressBar_all->setValue(index+1);
    ui->progressBar_current_label->setText("Текущий: Проверяем " + existsFiles[index]["Pbo"]);
}

// Начата загрузка обновления - оповещаем UI
void launcher::downloadUpdateStartUI() {

    qDebug() << "Debug-updateAddons_UI: download Update Start - UI";

    // Отключаем/включаем функционал
    ui->stopUpdater->setEnabled(true);
    stopUpdaterInProcess = true;
    ui->repoDisconnect->setEnabled(false);
    ui->downloadUpdate->setEnabled(false);
    ui->delOtherFiles->setEnabled(false);

    // Получаем список ID файлов которые выбраны для скачивания
    QList<int> fileID;
    int count = correctFiles.size() + notCorrectFiles.size() + newFiles.size();
    for(int i = correctFiles.size(); i<count;i++) {
        QTreeWidgetItem *item = ui->filesTree->topLevelItem(i);
        if(item->checkState(0) == Qt::Checked) {
            fileID.append(i-correctFiles.size());
        }
    }

    // Оповещаем UI, что загрузка начата
    ui->str1->setText("Загрузка аддонов начата");
    ui->progressBar_all_label->setText("Всего: Скачиваем аддоны");
    ui->progressBar_all_label2->setText("0/" + QString::number(fileID.size()));
    ui->progressBar_all->setMaximum(fileID.size());
    ui->progressBar_all->setValue(0);

    emit downloadUpdateStart(fileID);
}

// Начата загрузка нового файла - оповещаем UI
void launcher::downloadAddonStartedUI(const QList< QMap<QString, QString> > downloadFiles, int index) {

    qDebug() << "Debug-updateAddons_UI: download Addon Started - UI";

    // Считаем сколько всего нужно скачать и сколько скачали
    qlonglong totalSize = 0;
    qlonglong currentSize = 0;
    //..всего скачать
    for(int i = 0; i < downloadFiles.size();i++) {
        totalSize += downloadFiles[i]["Size"].toLongLong();
    }
    //..всего скачали
    for(int i = 1; i<= index;i++) {
        currentSize += downloadFiles[i-1]["Size"].toLongLong();
    }

    // Регистрируем прогресс в прогресс баре
    ui->progressBar_all->setMaximum(downloadFiles.size());
    ui->progressBar_all->setValue(index+1);

    // Описываем сколько осталось до конца загрузки из скольки, приводя к разумным типам исчесления данных
    // Если байты не больше..
    //..1 КБ
    if(totalSize < 1024)
        ui->progressBar_all_label2->setText(QString::number(currentSize)  + "/"
                                          + QString::number(totalSize) + " байт");
    //..1 МБ
    else if (totalSize < 1048576)
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1024))  + "/"
                                          + QString::number((int)(totalSize/1024)) + " КБ");
    //..ГБ
    else if (totalSize < 4294967296)
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1048576))  + "/"
                                          + QString::number((int)(totalSize/1048576)) + " МБ");
    // Если больше или равны 4 ГБ
    else
        ui->progressBar_all_label2->setText(QString::number((int)(currentSize/1048576))  + " МБ/"
                                          + QString::number((int)(totalSize/4294967296)) + " ГБ");
}

// Завершена загрузка файлов - оповещаем UI
void launcher::downloadAddonsFinishUI(bool success) {
    // Заполняем UI в соответсвии с успешностью загрузки
    ui->progressBar_all->setMaximum(1);
    ui->progressBar_all->setValue(1);
    if(!success) {
        ui->str1->setText("Загрузка аддонов остановлена");
        ui->progressBar_all_label->setText("Всего: Загрузка аддонов остановлена");
    } else {
        ui->str1->setText("Загрузка аддонов успешна");
        ui->progressBar_all_label->setText("Всего: Загрузка аддонов прошла успешно");
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
}

// Слот завершения удаления лишних файлов
void launcher::deleteOtherFilesFinish() {
    ui->delOtherFiles->setEnabled(false);
    ui->checkAddons->setEnabled(true);
}

// Меняем checkState цепочки файлов
void launcher::addonsTreeCheck(QTreeWidgetItem *item) {

    qDebug() << "Debug-updateAddons_UI: addonsTree - item changed " << item->text(0);

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
void launcher::on_md5check_stateChanged(int stateCheck) {

    qDebug() << "Debug-updateAddons_UI: MD5 show/hide";

    // Если checkState = Uncheked, то отображаем лишь файлы
    if(stateCheck == Qt::Unchecked)
        ui->filesTree->setColumnCount(1);
    else {// Иначе, то отображаем и МД5
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

    qDebug() << "Debug-updateAddons_UI: download File UI - start";

    // Оповещаем UI какой файл мы скачиваем
    ui->progressBar_current_label->setText("Текущий: Скачиваем " + fileName);

    // Включаем кнопку остановк загрузки
    ui->stopUpdater->setEnabled(true);
    stopUpdaterInProcess = true;

    qDebug() << "Debug-updateAddons_UI: download File UI - succ";
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
                                              + QString::number(totalBytes) + " байт");
    //..1 МБ
    else if (totalBytes < 1048576)
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1024))  + "/"
                                              + QString::number((int)(totalBytes/1024)) + " КБ");
    //..ГБ
    else if (totalBytes < 1073741824)
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1048576))  + "/"
                                              + QString::number((int)(totalBytes/1048576)) + " МБ");
    // Если больше или равны 1 ГБ
    else
        ui->progressBar_current_label2->setText(QString::number((int)(bytesRead/1048576))  + " МБ/"
                                              + QString::number((int)(totalBytes/1073741824)) + " ГБ");
}

// Обновлении данных в UI о завершении загрузки файла
void launcher::downloadFinished(bool success) {

    qDebug() << "Debug-updateAddons_UI: download Finished UI - start";

    // Отключаем кнопку остановки
    ui->stopUpdater->setEnabled(false);

    // Проверка успешной загрузки файла
    if(success)             // Если файл скачен успешно
        ui->progressBar_current_label->setText(ui->progressBar_current_label->text().replace("Скачиваем", "Скачен"));
    else                    // Если нет
        ui->progressBar_current_label->setText("Текущий: Скачивание остановлено");

    qDebug() << "Debug-updateAddons_UI: download Finished UI - succ";
}

/*
 * Слоты unzip'а
 */
// Обновлении данных в UI о начале распаковки
void launcher::unzipStart(QString fileName) {

    qDebug() << "Debug-updateAddons_UI: unzip Start UI - start";

    // Оповещаем пользователя, что началась распаковка
    ui->progressBar_current_label->setText("Текущий: Распаковываем " + fileName);
    ui->progressBar_current->setValue(0);
    ui->progressBar_current->setMaximum(1);

    qDebug() << "Debug-updateAddons_UI: unzip Start UI - succ";
}

// Обновлении данных в UI о завершении распаковки
void launcher::unzipFinished(QString fileName) {

    qDebug() << "Debug-updateAddons_UI: unzip Finished UI - start";

    // Оповещаем пользователя, что распаковка закончилась
    ui->progressBar_current_label->setText("Текущий: Распаковали " + fileName);
    ui->progressBar_current->setValue(1);
    ui->progressBar_current->setMaximum(1);

    qDebug() << "Debug-updateAddons_UI: unzip Finished UI - succ";
}
