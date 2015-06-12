#-------------------------------------------------
#
# Project created by QtCreator 2015-04-28T15:45:44
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT       += network

TARGET = ArmALauncher
TEMPLATE = app
RC_FILE += resources.rc
OTHER_FILES += resources.rc \ $$basename(TARGET).manifest \

# Для релиза
CONFIG(release, debug|release) {
    WINSDK_DIR = C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.0A\\Bin # путь к исполняемым файлам Windows SDK (с mt.exe)
    WIN_PWD = $$replace(PWD, /, \\) # заменить все '/' на '\\' в строке пути к исполняемому файлу программы
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
    QMAKE_POST_LINK = \"$$WINSDK_DIR\\mt.exe\" -manifest \"$$WIN_PWD\\$$basename(TARGET).manifest\" -outputresource:\"$$OUT_PWD_WIN\\${DESTDIR_TARGET}\"\;1 # вызов программы mt.exe после компиляции приложения
    QMAKE_LFLAGS += -static -static-libgcc
    #DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += main.cpp\
        launcher.cpp \
    favoriteservers.cpp \
    updateinformation.cpp \
    serveredit.cpp \
    xmlparser.cpp \
    updateaddons.cpp \
    updateaddons_ui.cpp \
    deleteotherfiles.cpp \
    addonssettings.cpp \
    launchersettings.cpp \
    repoedit.cpp \
    launcherupdate.cpp

HEADERS  += launcher.h \
    cfg.h \
    version.h \
    serveredit.h \
    xmlparser.h \
    updateaddons.h \
    deleteotherfiles.h \
    addonssettings.h \
    launchersettings.h \
    repoedit.h \
    launcherupdate.h

FORMS    += launcher.ui \
    serveredit.ui \
    deleteotherfiles.ui \
    addonssettings.ui \
    launchersettings.ui \
    repoedit.ui \
    launcherupdate.ui

RESOURCES += pictures.qrc \
    style.qrc \
    lang.qrc

LIBS +=libversion

TRANSLATIONS += launcher_eu.ts
