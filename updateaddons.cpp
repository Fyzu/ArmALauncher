#include "launcher.h"
#include "ui_launcher.h"


// Конструктор
updateAddons::updateAddons(QWidget *parent) : QObject(parent) {

    qDebug() << "Debug-updateAddons: Updater constructor";

    // Объявление мета типов
    qRegisterMetaType<Repository>("Repository");
    qRegisterMetaType< QList< QMap<QString, QString> > >("QList< QMap<QString, QString> >");
    qRegisterMetaType<QStringList>("QStringList");
    qRegisterMetaType< QList<int> >("QList<int>");
}

// Диструктор
updateAddons::~updateAddons() {

    qDebug() << "Debug-updateAddons: Updater destructor";

    emit finished();
}

// Сохранение временных данных о файлах
void updateAddons::saveTempFileInfo() {

    QFile file(QCoreApplication::applicationDirPath()+"/temp/fileList.temp");
    if(file.open(QIODevice::WriteOnly))  {  //Если файл открыт успешно
        QDataStream out(&file);             //Создаем поток для записи данных в файл

        //В поток
         out << yomaFileInfo << syncFileInfo;;

         qDebug() << "Debug-updateAddons: Save Temp File Info in file - succ";
    } else {
        qDebug() << "Debug-updateAddons: Save Temp File Info in file - fail";
    }
}

// Прописываем данные репозитория
void updateAddons::setRepository(Repository repo) {

    qDebug() << "Debug-updateAddons: set new Reposirory";

    // Получаем параметры репозитория
    repository = repo;
    if(repository.type == 0) {
        QFileInfo info(repository.url);
        pathUrl = repository.url;
        pathUrl.remove(info.fileName());
    } else {
        repository.url.replace("/autoconfig", "/sync");
        pathUrl = repository.url;
        pathUrl.remove(".a3s/sync");
    }
    // Сброс данных
    addonsList.clear();
    modsList.clear();
    addonsFolders.clear();
    addonsPath.clear();
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();
    downloadFiles.clear();
    downloadTime = 0;
    bytesBefore = 0;
    url = 0;
    manager = 0;
    reply = 0;
    filePath = 0;
    httpRequestAborted = false;
    downloadStart = false;
    downloadAddonsInProgress = false;
    downloadAddonsAbort = false;
    downloadFilesIndex = 0;

    // Считываем список файлов
    //..считывание
    QFile file(QCoreApplication::applicationDirPath()+"/temp/fileList.temp");
    if(file.open(QIODevice::ReadOnly)) {

        //открываем поток ввода
        QDataStream in(&file);

        //Из потока
        in >> yomaFileInfo >> syncFileInfo;
        file.close();

        qDebug() << "Debug-updateAddons: Load Temp File Info in file - succ";
    } else {
        yomaFileInfo.clear();
        syncFileInfo.clear();
        qDebug() << "Debug-updateAddons: Load Temp File Info in file - fail";
    }
}

// Старт класса updateAddons
void updateAddons::start() {

    qDebug() << "Debug-updateAddons: Updater start";

    // Подключаем слот  к сигналу завершения загрузки, который вызовит метод завершающий запус класса
    connect(this, SIGNAL(downloadFinished(bool)), this, SLOT(startFinished(bool)), Qt::DirectConnection);
    // Скачиваем конфиг
    downloadFile(repository.url, QCoreApplication::applicationDirPath() + "/temp/");
}

// Завершающий этап запуска программы
void updateAddons::startFinished(bool success) {

    qDebug() << "Debug-updateAddons: Updater start - Finished stage";

    // Проверяем, успешно ли скачен файл
    if(!success) {
        qDebug() << "Debug-updateAddons: Connect not success - file not download";
        emit finished();
        return;
    }

    // Получение путей и информации о файле
    QFileInfo info(repository.url);
    QString path = QCoreApplication::applicationDirPath() + "/temp/";

    // Парсим информацю
    if(repository.type == 0) {
        parseYomaInformation(path, info.fileName());
    } else {
        parseSyncInformation(path);
    }
    qDebug() << "Debug-updateAddons: Updater start - Finished stage - succ";

    // Операции после успешной
    disconnect(this, SIGNAL(downloadFinished(bool)), this, SLOT(startFinished(bool)));
    emit started(repository, addonsList, modsList);
}

void updateAddons::parseYomaInformation(QString path, QString fileName) {

    // Распаковка архива
    unzipArchive(path+fileName, path);

    // Парсим информацю
    qDebug() << "Debug-updateAddons: Updater start - Finished stage - parsing start";
    XMLParser parser(path + "Addons.xml");
    addonsList = parser.getAddons();
    parser.setPath(path + "Mods.xml");
    modsList = parser.getMods();

    // Удаление считанных файлов
    QFile::remove(path+"Addons.xml");
    QFile::remove(path+"Mods.xml");
    QFile::remove(path+"Server.xml");
    QFile::remove(path+"Addons.xsd");
    QFile::remove(path+"Mods.xsd");
    QFile::remove(path+"Server.xsd");
    QString name = fileName;
    name.remove(".7z");
    QFileInfo folder(path+name);
    folder.dir().rmdir(folder.absoluteFilePath());

    qDebug() << "Debug-updateAddons: Updater start - Finished stage - parsing finish";
}

void updateAddons::parseSyncInformation(QString path) {

    qDebug() << "Debug-updateAddons: Updater start - Finished stage - parsing start";

    // Парсим инфу аддон синка
    if(QFile::exists(path+"sync")) {
        // Запускаем парсер
        QProcess sync(this);
        sync.start("ArmALauncher-SyncParser.exe 0 \"" + path + "sync\"");
        //..ожидаем завершение парсера
        sync.waitForFinished(999999);
        sync.deleteLater();
        // Читаем вывод парсера
        QStringList list;
        QFile inputfile(path + "sync.txt");
        if (inputfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
           QTextStream in(&inputfile);
           while (!in.atEnd())
                list.append(in.readLine());

           inputfile.close();
        }

        // Очищаем данные
        addonsList.clear();
        modsList.clear();

        // Получаем список модов
        int index;
        for(index = 0; index<list.size() && list[index] != " - - sha1 - - ";index++) {
            modsList.append(list[index]);
            qDebug() << list[index];
        }
        // Получаем список файлов и их атрибуты
        index++;
        while(index<list.size()) {
            QMap<QString, QString> file;
            file.insert("Path", list[index]);
            file.insert("Pbo",  list[index+1]);
            file.insert("Sha1", list[index+2]);
            file.insert("Size", list[index+3]);
            addonsList.append(file);
            index+=4;
        }

        // Удаляем считанные файлы
        QFile::remove(path + "sync");
        QFile::remove(path + "sync.txt");
    }

    qDebug() << "Debug-updateAddons: Updater start - Finished stage - parsing finish";
}

// Получение результатов после старта UI апдейтера
void updateAddons::updaterUIStarted(QStringList folders) {

    qDebug() << "Debug-updateAddons: updater UI started";

    addonsFolders = folders;
}

// Завершение работы апдейтера
void updateAddons::finish() {

    downloadTime = 0;
    bytesBefore = 0;
    url.clear();
    manager = 0;
    reply = 0;
    filePath = 0;
    httpRequestAborted = false ;
    downloadStart = false;
    addonsList.clear();
    modsList.clear();
    addonsFolders.clear();
    addonsPath.clear();
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();
    downloadFiles.clear();
    downloadAddonsInProgress = false;
    downloadAddonsAbort = false;
    pathUrl.clear();
    downloadFilesIndex=0;
    yomaFileInfo.clear();
    syncFileInfo.clear();

    qDebug() << "Debug-updateAddons: updater Finish";
}

/*
 * Функции работы с репозиторем
 */
// Проверить аддоны
void updateAddons::checkAddons(QString path) {

    qDebug() << "Debug-updateAddons: check Addons - start";

    // Получаем путь к аддонам
    addonsPath = path;

    // Временные переменные
    QList< QMap<QString, QString> > allFiles;
    QList< QMap<QString, QString> > existsFiles;

    // Очищаем данные, для заполнения свежими данными
    otherFiles.clear();
    newFiles.clear();
    correctFiles.clear();
    notCorrectFiles.clear();

    // Получаем список всех файлов которые находятся в аддонах
    for(int folder = 0; folder<addonsFolders.count(); folder++) {
        getAllFilesInDir(addonsPath+"/"+addonsFolders[folder], allFiles);
    }

    // Получаем списки файлов которые есть в списке сервера и те которые являются лишними
    for(int i = 0; i < allFiles.size();i++) {
        int j;
        for(j = 0; j < addonsList.size(); j++) {
            if(QString::compare(allFiles[i]["Pbo"],  addonsList[j]["Pbo"], Qt::CaseInsensitive) == 0 &&
               QString::compare(allFiles[i]["Path"], addonsList[j]["Path"], Qt::CaseInsensitive) == 0) {
                existsFiles.append(addonsList[j]);
                break;
            }
        }
        if(j == addonsList.size()                   && !allFiles[i]["Pbo"].contains(".wav") &&
           !allFiles[i]["Pbo"].contains(".dll")     && !allFiles[i]["Pbo"].contains(".txt") &&
           !allFiles[i]["Pbo"].contains(".bikey")   && !allFiles[i]["Pbo"].contains(".bisign") &&
           !allFiles[i]["Pbo"].contains(".cpp")     && !allFiles[i]["Pbo"].contains(".paa"))
            // добавляем файл на удаление
            otherFiles.append(allFiles[i]);
    }

    // Получаем список новых файлов
    newFiles = addonsList;
    for(int index = 0; index < existsFiles.size(); index++) {
        newFiles.removeAll(existsFiles[index]);
    }

    // Получаем списки корректных файлов и нет
    if(repository.type == 0)
    // Yoma Addon Sync 2009
        for(int index = 0; index<existsFiles.size(); index++) {
            // Регистрируем прогресс в UI
            emit checkAddonsProgressUI(index, existsFiles);

            // Получаем информацию о файле
            QFileInfo info(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);

            // Проверяем, есть ли файл в заранее собранном файле
            YomaFileInfo temp;
            temp.fileName = addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"];
            int tempIndex = yomaFileInfo.indexOf(temp);


            // Проверяем, схожа ли заранее подготовленая информация о файле с действительностью
            if(tempIndex != -1) {           // Если файл найден
                if(info.lastModified() == yomaFileInfo[tempIndex].fileEditDate && info.size() == yomaFileInfo[tempIndex].fileSize
                  && QString::compare(yomaFileInfo[tempIndex].fileMD5, existsFiles[index]["Md5"], Qt::CaseInsensitive) == 0) {             // Проверяем, действительная ли информация о файле
                    correctFiles.append(existsFiles[index]);
                    continue;
                } else {        // Если информация не действительна
                    yomaFileInfo.removeAt(tempIndex);
                }
            }

            // Получаем MD5 сумму файла
            QString md5 = getChecksum(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"], QCryptographicHash::Md5);
            // Проверяем, соответсвует ли сумма на сервере, сумме локальной
            if(QString::compare(md5, existsFiles[index]["Md5"], Qt::CaseInsensitive) == 0) {
                YomaFileInfo fileInfo;
                fileInfo.fileName = (addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);
                fileInfo.fileMD5 = (existsFiles[index]["Md5"]);
                fileInfo.fileEditDate = (info.lastModified());
                fileInfo.fileSize = (info.size());
                yomaFileInfo.append(fileInfo);
                correctFiles.append(existsFiles[index]);
            } else {
                notCorrectFiles.append(existsFiles[index]);
                notCorrectFiles.last().insert("Md5local", md5.toUpper());
            }
        }
    else
    // Arma3 Sync
        for(int index = 0; index<existsFiles.size(); index++) {
            // Регистрируем прогресс в UI
            emit checkAddonsProgressUI(index, existsFiles);

            // Получаем информацию о файле
            QFileInfo info(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);

            // Проверяем, есть ли файл в заранее собранном файле
            SyncFileInfo temp;
            temp.fileName = addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"];
            int tempIndex = syncFileInfo.indexOf(temp);


            // Проверяем, схожа ли заранее подготовленая информация о файле с действительностью
            if(tempIndex != -1) {           // Если файл найден
                if(info.lastModified() == syncFileInfo[tempIndex].fileEditDate && info.size() == syncFileInfo[tempIndex].fileSize
                  && QString::compare(syncFileInfo[tempIndex].fileSHA1, existsFiles[index]["Sha1"], Qt::CaseInsensitive) == 0) {             // Проверяем, действительная ли информация о файле
                    correctFiles.append(existsFiles[index]);
                    continue;
                } else {        // Если информация не действительна
                    syncFileInfo.removeAt(tempIndex);
                }
            }

            // Получаем MD5 сумму файла
            QString sha1 = getChecksum(addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"], QCryptographicHash::Sha1);
            if(sha1 == "da39a3ee5e6b4b0d3255bfef95601890afd80709")
                sha1 = "0";
            // Проверяем, соответсвует ли сумма на сервере, сумме локальной
            if(QString::compare(sha1, existsFiles[index]["Sha1"], Qt::CaseInsensitive) == 0) {
                SyncFileInfo fileInfo;
                fileInfo.fileName = (addonsPath+"/"+existsFiles[index]["Path"]+"/"+existsFiles[index]["Pbo"]);
                fileInfo.fileSHA1 = (existsFiles[index]["Sha1"].toUpper());
                fileInfo.fileEditDate = (info.lastModified());
                fileInfo.fileSize = (info.size());
                syncFileInfo.append(fileInfo);
                correctFiles.append(existsFiles[index]);
            } else {
                notCorrectFiles.append(existsFiles[index]);
                notCorrectFiles.last().insert("Sha1local", sha1.toUpper());
            }
        }

    qDebug() << "Debug-updateAddons: check Addons - succ";

    // Передаем данные UI
    emit checkAddonsFinished(repository.type, otherFiles, newFiles, correctFiles, notCorrectFiles);

    // Сохраняем информацию о файлах
    saveTempFileInfo();
}

// Рекурсивный метод, получения списка всех файлов в папке и подпапках
void updateAddons::getAllFilesInDir(const QString path, QList< QMap<QString, QString> > &allFiles) {

    // Получаем файлы в текущей папке path
    QStringList files = QDir(path).entryList(QDir::Files);
    for(int index = 0; index<files.size();index++) {
        QMap<QString, QString > file;
        QString addonPath = path;
        file.insert("Path", addonPath.remove(addonsPath+"/").replace('/', '\\'));
        file.insert("Pbo", files[index]);
        allFiles.append(file);
    }

    // Получаем папки в текущей позиции Path и очищаем от мусора
    QStringList folders = QDir(path).entryList(QDir::Dirs);
    for(int j=0;j<folders.size();j++) {
        if(folders.at(j) == "." || folders.at(j) == "..") {
            folders.removeAt(j);
            j--;
        }
     }

    // Вызываем этот же метод, на все папки в текущей позиции path
    for(int index = 0;index<folders.size();index++) {
        getAllFilesInDir(path+"/"+folders[index], allFiles);
    }
}

// Скачать/обновить аддоны
void updateAddons::downloadUpdate(QList<int> fileID) {

    qDebug() << "Debug-updateAddons: download Update - start";

    // Собираем список скачиваемых файлов
    downloadFiles.clear();
    for(int id = 0;id<fileID.size();id++) {
        if(fileID[id] >= notCorrectFiles.size())
            downloadFiles.append(newFiles[fileID[id]-notCorrectFiles.size()]);
        else
            downloadFiles.append(notCorrectFiles[fileID[id]]);
    }

    // Если нет файлов для скачивания
    if(downloadFiles.size() == 0) {
        downloadAddonsFinish();
        emit downloadAddonsFinished(true);
        return;
    }

    // Переменные загрузки
    downloadFilesIndex = 0;
    downloadAddonsInProgress = true;
    downloadAddonsAbort = false;
    // связь, конец загрузки аддонов
    connect(this, SIGNAL(downloadAddonsFinished(bool)),
            this, SLOT  (downloadAddonsFinish()));
    // Запускаем первый этап загрузки
    downloadAddonStart();

    qDebug() << "Debug-updateAddons: download Update - succ";
}

// Загрузка файла - первая стадия загрузки
void updateAddons::downloadAddonStart() {

    qDebug() << "Debug-updateAddons: download Addon Start Stage - start";

    // связываем след этап, с концом загрузки файла
    connect(this, SIGNAL(downloadFinished(bool)), this, SLOT(downloadAddonFinish(bool)), Qt::QueuedConnection);
    //..загружаем файл
    if(repository.type == 0)
        downloadFile(pathUrl + downloadFiles[downloadFilesIndex]["Url"], QCoreApplication::applicationDirPath() + "/temp/");
    else
        downloadFile(pathUrl + downloadFiles[downloadFilesIndex]["Path"] + "/" + downloadFiles[downloadFilesIndex]["Pbo"],
                     addonsPath + "/" + downloadFiles[downloadFilesIndex]["Path"] + "/");

    // Даем сигнал UI, что начата загрузка downloadFilesIndex файла
    emit downloadAddonStarted(downloadFiles, downloadFilesIndex);

    qDebug() << "Debug-updateAddons: download Addon Start Stage - succ";
}

// Загрузка файла - финальная стадия загрузки
void updateAddons::downloadAddonFinish(bool succ) {

    qDebug() << "Debug-updateAddons: download Addon Finish Stage - start";

    // Отвязываем конец загрузки от последнего этапа загрузки
    disconnect(this, SIGNAL(downloadFinished(bool)), this, SLOT(downloadAddonFinish(bool)));

    // Распаковка скаченного архива
    if(succ) {
        if(repository.type == 0) { // Если загрузка завершена успешно
            // Получение путей и информации о файле
            QFileInfo info(downloadFiles[downloadFilesIndex]["Url"]);
            unzipArchive(QCoreApplication::applicationDirPath() + "/temp/" + info.fileName(),
                     addonsPath + "/" + downloadFiles[downloadFilesIndex++]["Path"]);
        } else {
            downloadFilesIndex++;
        }
    }
    // Выбор решения по результатам загрузки
    //..если загрузка завершилась не успешно или она остановлена
    if((!succ || downloadAddonsAbort) && downloadFilesIndex != downloadFiles.size()) {
        qDebug() << "Debug-updateAddons: Abort download Addons";
        downloadAddonsFinish();
        emit downloadAddonsFinished(false);
    }
    //..если загрузка завершена успешно
    else if(succ && downloadFilesIndex == downloadFiles.size()) {
        downloadAddonsFinish();
        emit downloadAddonsFinished(true);
    //..иначе, файл загружен успешно
    } else if(succ) {
        // Загрузка следующего файла
        downloadAddonStart();
    }

    qDebug() << "Debug-updateAddons: download Addon Finish Stage - succ";
}

// Завершение загрузки файлов
void updateAddons::downloadAddonsFinish() {

    qDebug() << "Debug-updateAddons: download Addons Finish - start";

    downloadAddonsInProgress = false;

    disconnect(this, SIGNAL(downloadAddonsFinished(bool)),
               this, SLOT  (downloadAddonsFinish()));

    qDebug() << "Debug-updateAddons: download Addons Finish - succ";
}

// Удалить лишние файлы
void updateAddons::delOtherFiles() {

    qDebug() << "Debug-updateAddons: del Other Files";

    emit deleteOtherFiles(otherFiles, addonsPath);
}

// Остановить обновление аддонов
void updateAddons::stopUpdater() {

    qDebug() << "Debug-updateAddons: stop Updater - start";

    downloadAddonsAbort = true;

    // Останавливаем загрузку
    if(reply) { // Если процесс загрузки идет
        
        qDebug() << "Debug-httpDownload: Download stop";

        // Заканчиваем загрузку
        httpRequestAborted = true;
        // Отключаем сигнал и вызвапем метод вручную
        disconnect(reply, SIGNAL(finished()),
                   this,  SLOT  (httpDownloadFinished()));
        disconnect(reply, SIGNAL(readyRead()),
                   this,  SLOT  (httpReadyRead()));
        disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                   this,  SLOT  (updateDownloadProgress(qint64,qint64)));
        //.. регистрируем завершение загрузки
        reply->abort();
        httpDownloadFinished();
    }
}

// Метод загрузки файла по Урл в нужную директорию
void updateAddons::downloadFile(QString fileUrl, QString path) {

    qDebug() << "Debug-updateAddons: download File - " << fileUrl;

    // Создаем менеджр сети
    manager = new QNetworkAccessManager;

    // Получаем url файла
    url.setUrl(fileUrl.replace('\\', '/'));

    // Получаем имя файла и проверяем его корректность
    QDir().mkpath(path);
    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        qDebug() << "Debug-updateAddons: File name - empty";
        return;
    }

    // Прописываем полный путь файл
    filePath = new QFile(path+fileName);
    qDebug() << "Debug-updateAddons: File download in - " << path;
    if (filePath->exists()) {
        qDebug() << "Debug-updateAddons: File exists - " << filePath->fileName() << " - remove";
        filePath->remove();
    }
    if (!filePath->open(QIODevice::WriteOnly)) {
        qDebug() << "Debug-updateAddons: File open - fail";
        delete filePath;
        filePath = 0;
        return;
    }

    // Прописываем информацию в наш processBar_current
    httpRequestAborted = false;
    downloadStart = false;

    // Подготавливаемся считать скорость загрузки
    downloadTime=QDateTime::currentMSecsSinceEpoch();
    bytesBefore = 0;
    triger = 4;

    // Запускаем запрос загрузки
    startRequest(url);

    // Передаем сигнал UI для обновление данных
    emit downloadFile_UI(fileName);
}

// Запрос загрузки и подключение сигналов
void updateAddons::startRequest(QUrl url) {

    qDebug() << "Debug-updateAddons: start Request - " << url;

    // Посылаем запрос по URL
    reply = manager->get(QNetworkRequest(url));

    //..и готовимся считывать данные по сигналу readyRead
    connect(reply, SIGNAL(readyRead()),
            this,  SLOT  (httpReadyRead()), Qt::DirectConnection);
    //..так же, регистрируем прогресс загрузки
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this,  SLOT  (updateDownloadProgress(qint64,qint64)), Qt::DirectConnection);
    //..а по завершению загрузки, взываем метод httpDownloadFinished()
    connect(reply, SIGNAL(finished()),
            this,  SLOT  (httpDownloadFinished()), Qt::DirectConnection);

}

// Метод вызывается когда QNetworkReply готов к считыанию данных
void updateAddons::httpReadyRead() {

    // Считываем входные данные в файл
    if (filePath)
        filePath->write(reply->readAll());
}

// Метод обновления данных загрузки
void updateAddons::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes) {

    // Обновляем данные загрузки
    if (!httpRequestAborted) {
        downloadStart = true;
        QString speed;
        if(triger == 4) {
            triger = 0;
            // Расчитываем скорость загрузки в кб/сек
            qint64 time = QDateTime::currentMSecsSinceEpoch();
            unsigned int bytes = static_cast<unsigned int>((bytesRead - bytesBefore));
            unsigned int timeRes = static_cast<unsigned int>((time - downloadTime));
            downloadTime = time;
            bytesBefore = bytesRead;
            speed = QString::number((int) (( bytes * 1000) /( (timeRes+1) * 1024))) + " КБ/сек";
        } else
            triger++;

        // Передаем сигнал UI для обновление данных
        emit updateDownloadProgress_UI(bytesRead, totalBytes, speed);
    }
}

// Метод вызвается когда загрузка завершена
void updateAddons::httpDownloadFinished() {

    qDebug() << "Debug-updateAddons: http Download Finished";

    // Если нажали кнопку "Остановить загрузку"
    if (httpRequestAborted) {
        // Удаляем недокаченный файл
        if (filePath) {
            filePath->close();
            filePath->remove();
            filePath->deleteLater();
            filePath = 0;
        }
        // Собираем мусор
        manager->deleteLater();
        manager = 0;
        reply->deleteLater();
        reply = 0;

        // Передаем сигнал UI для обновление данных
        emit downloadFinished(false);
        return;
    }

    // Загрузка прошла успешно
    filePath->flush();
    filePath->close();
    // Проверка возникали ли проблемы в загрузке
    if (reply->error()) {
        filePath->remove();
        qDebug() << "Debug-updateAddons: Download failed: " << reply->errorString();
    } else
       qDebug() << "Debug-updateAddons: Download succ - " << filePath->exists();

    // Собираем мусор
    reply->deleteLater();
    reply = 0;
    filePath->deleteLater();
    filePath = 0;
    manager->deleteLater();
    manager = 0;

    // Передаем сигнал UI для обновление данных
    emit downloadFinished(true);
}

// Получаем проверочную сумму файла по пути
QString updateAddons::getChecksum(QString filePath, QCryptographicHash::Algorithm alg) {

    QFile file(filePath);
    QCryptographicHash crypto(alg);
    if(file.open(QIODevice::ReadOnly)) {
        while(!file.atEnd()){
          crypto.addData(file.read(1024 * 32));
        }
        file.close();
        return crypto.result().toHex().constData();
    }
    return "";
}

// Распокавать архив в указанное место
bool updateAddons::unzipArchive(QString archive, QString extracting) {
    if(!archive.isEmpty() && !extracting.isEmpty()) {

        qDebug() << "Debug-updateAddons: unzip - start";

        // Оповещаем пользователя, что распаковка началась
        QFileInfo info(archive);
        if(QFile::exists(archive))
            emit unzipStart(info.fileName());

        // Распаковываем файли используя консольную версию 7z
        // e - параметр распковки, "путь к архиву",  -o"путь куда",  -mmt многопоточность, -y соглашаться на все
        QProcess unzip(this);
        unzip.start("\"" + QCoreApplication::applicationDirPath() + "/" + "7z.exe\" e \"" + archive + "\" -o\"" + extracting + "\" -mmt -y");
        unzip.waitForFinished(99999);

        // Получаем вывод 7z, для удобного дебага
        qDebug() << "Debug-updateAddons: 7z output: " << unzip.readAll();
        unzip.deleteLater();

        // Завершение распаковки
        //..удаление архива
        QFile::remove(archive);
        //..оповещаем пользователя, что распаковка удалась
        if(QFile::exists(extracting + "/" + info.fileName().remove(".7z")))
            emit unzipFinished(info.fileName());

        qDebug() << "Debug-updateAddons: unzip - succ";
        return true;
    }
    return false;
}
