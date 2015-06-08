#include <QStringList>
#include <QDateTime>

namespace Ui {
    class launcher;
}

// Структура данных Избранного сервера
struct favServer{

    // Параметры сервера
    QString serverName;
    QString serverIP;
    QString serverPort;
    QString serverPass;
    QStringList serverAddons;
    bool check_name;
    QString name;

    // Онлайн информация о сервере
    QString ping;
    QString GameVersion;
    QString HostName;
    QString MapName;
    QString GameType;
    QString NumPlayers;
    QString NumTeam;
    QString MaxPlayers;
    QString GameMode;
    QString TimeLimit;
    QString Password;
    QString CurrentVersion;
    QString RequiredVersion;
    QString Mod;
    QString BattleEye;
    QString Longitude;
    QString Latitude;
    QString Mission;

    // Конструктор сервера
    favServer() {
        serverName = "Новый сервер";
        serverIP = "127.0.0.1";
        serverPort = "2302";
        check_name = false;
        name = "";
        noResponse();
    }

    favServer& operator=(const favServer& other) {
        serverName = other.serverName;
        serverIP = other.serverIP;
        serverPort = other.serverPort;
        serverPass = other.serverPass;
        serverAddons = other.serverAddons;
        check_name = other.check_name;
        name = other.name;
        return *this;
    }

    // Если нет ответа от сервера
    void noResponse() {
        ping.clear();
        GameVersion.clear();
        HostName.clear();
        MapName.clear();
        GameType.clear();
        NumPlayers.clear();
        NumTeam.clear();
        MaxPlayers.clear();
        GameMode.clear();
        TimeLimit.clear();
        Password.clear();
        CurrentVersion.clear();
        RequiredVersion.clear();
        Mod.clear();
        BattleEye.clear();
        Longitude.clear();
        Latitude.clear();
        Mission.clear();
    }
};

// Структура данных параметров аддона
struct addon {
    QString addonName;
    QString addonDir;
    QString addonPath;

    addon (QString aN, QString aD, QString aP) {
        addonName = aN;
        addonDir = aD;
        addonPath = aP;
    }
};

// Переменные настроек игры
struct param {
    // Дополнительные параметры
    QString addParam;
    // Настройки игры
    bool check_name;
    QString name;
    bool window;
    bool noPause;
    bool showScriptErrors;
    bool noFilePatching;
    bool battlEye;
    // Настройки производительности
    int priorityLaunch;
    bool check_maxMem;
    QString maxMem;
    bool check_maxVRAM;
    QString maxVRAM;
    bool check_cpuCount;
    QString cpuCount;
    bool check_exThreads;
    QString exThreads;
    bool check_malloc;
    QString malloc;
    bool enableHT;
    bool winxp;
    bool noCB;
    bool nosplash;
    bool skipIntro;
    bool worldEmpty;
    bool noLogs;
};

// Структура данных диагностики DirectX
struct dxdiag {
    /*
     * SystemInformation
     */
    QString OperatingSystem;
    QString Processor;
    QString Memory;
    QString DirectXVersion;
    /*
     * DisplayDevice
     */
    QString CardName;
    QString SharedMemory;
};

// Структура репозитория
struct Repository {
    QString name;
    QString url;
    int type;

    Repository() {
        name = "Новый репозиторий";
        url = "";
        type = 0;
    }
};

// Настройки лаунчера
struct Settings {
    // Стиль
    int style;
    bool documentMode;

    // Запуск
    int launch;

    Settings() {
        style = 0;
        documentMode = false;
        launch = 0;
    }
};

struct YomaFileInfo {

    QString     fileName;
    qint64      fileSize;
    QDateTime   fileEditDate;
    QString     fileMD5;

    bool operator==(const YomaFileInfo& other) {
        return fileName == other.fileName;
    }
};

struct SyncFileInfo {

    QString     fileName;
    qint64      fileSize;
    QDateTime   fileEditDate;
    QString     fileSHA1;

    bool operator==(const SyncFileInfo& other) {
        return fileName == other.fileName;
    }
};

/*
 * Перегрузка операций добавления\изъятия данных потока.
 */
inline QDataStream & operator << ( QDataStream & out, const favServer & info ) {

    out << info.serverName << info.serverIP << info.serverPort << info.serverPass << info.serverAddons << info.check_name << info.name;
    return out;
}
inline QDataStream & operator >> (QDataStream &in, favServer &info) {

    in >> info.serverName >> info.serverIP >> info.serverPort >> info.serverPass >> info.serverAddons >> info.check_name >> info.name;

    return in;
}
inline QDataStream & operator << ( QDataStream & out, const param & info ) {

    out <<    info.check_name << info.name << info.window << info.noPause << info.showScriptErrors << info.noFilePatching << info.priorityLaunch <<
              info.check_maxMem << info.maxMem << info.check_maxVRAM << info.maxVRAM << info.check_cpuCount << info.cpuCount << info.check_exThreads <<
              info.exThreads << info.check_malloc << info.malloc << info.enableHT << info.winxp << info.noCB << info.nosplash << info.skipIntro <<
              info.worldEmpty << info.noLogs << info.battlEye << info.addParam;

    return out;
}
inline QDataStream & operator >> (QDataStream &in, param &info) {

    in >>   info.check_name >> info.name >> info.window >> info.noPause >> info.showScriptErrors >> info.noFilePatching >> info.priorityLaunch >>
            info.check_maxMem >> info.maxMem >> info.check_maxVRAM >> info.maxVRAM >> info.check_cpuCount >> info.cpuCount >> info.check_exThreads >>
            info.exThreads >> info.check_malloc >> info.malloc >> info.enableHT >> info.winxp >> info.noCB >> info.nosplash >> info.skipIntro >>
            info.worldEmpty >> info.noLogs >> info.battlEye >> info.addParam;

    return in;
}
inline QDataStream & operator << ( QDataStream & out, const Repository & info ) {

    out << info.name << info.url << info.type;

    return out;
}
inline QDataStream & operator >> (QDataStream &in, Repository &info) {

    in >> info.name >> info.url >> info.type;

    return in;
}
inline QDataStream & operator << ( QDataStream & out, const Settings & info ) {

    out << info.style << info.documentMode << info.launch;

    return out;
}
inline QDataStream & operator >> (QDataStream &in, Settings &info) {

    in >> info.style >> info.documentMode >> info.launch;

    return in;
}
inline QDataStream & operator << ( QDataStream & out, const YomaFileInfo & info ) {

    out << info.fileName << info.fileSize << info.fileEditDate << info.fileMD5;

    return out;
}
inline QDataStream & operator >> (QDataStream &in, YomaFileInfo &info) {

    in >> info.fileName >> info.fileSize >> info.fileEditDate >> info.fileMD5;

    return in;
}
inline QDataStream & operator << ( QDataStream & out, const SyncFileInfo & info ) {

    out << info.fileName << info.fileSize << info.fileEditDate << info.fileSHA1;

    return out;
}
inline QDataStream & operator >> (QDataStream &in, SyncFileInfo &info) {

    in >> info.fileName >> info.fileSize >> info.fileEditDate >> info.fileSHA1;

    return in;
}

