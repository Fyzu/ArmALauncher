#include "xmlparser.h"

XMLParser::XMLParser(QString XMLPath) {

    // Считываем данные XML файла
    QFile file(XMLPath);
    if(file.open(QIODevice::ReadOnly)) {
        xmlReader.addData(file.readAll());
        qInfo() << "XMLParser::XMLParser: xml open succ";
    } else {
        qInfo() << "XMLParser::XMLParser: xml open error - file: " << file.fileName();
    }
    file.close();
}

XMLParser::~XMLParser() { /* empty */}

// Загружаем новый файл в парсер
void XMLParser::setPath(QString path) {

    // Считываем данные XML файла
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        xmlReader.clear();
        xmlReader.addData(file.readAll());
        qInfo() << "XMLParser::setPath: xml open succ";
    } else {
        qInfo() << "XMLParser::setPath: xml open error - file: " << file.fileName();
    }
    file.close();
}

// Получить информацию из ХМЛки dxdiag'а
dxdiag XMLParser::getDxdiag() {

    qInfo() << "XMLParser::getDxdiag: xml parse start";

    dxdiag diag;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qWarning() << "XMLParser::getDxdiag: Parse error. " << xmlReader.errorString() << xmlReader.text();
            return diag;
        } else {
            switch (xmlReader.tokenType()) {
                case QXmlStreamReader::StartDocument:
                    continue;
                case QXmlStreamReader::StartElement: {
                    if (xmlReader.name() == "DxDiag")
                        continue;
                    if (xmlReader.name() == "SystemInformation") {
                        if (xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() == "SystemInformation")
                            break;

                        xmlReader.readNext();
                        while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "SystemInformation"))
                        {
                            if (xmlReader.tokenType() == QXmlStreamReader::StartElement) {
                                if(xmlReader.name() == "OperatingSystem") {
                                    xmlReader.readNext();
                                    diag.OperatingSystem = xmlReader.text().toString();
                                }
                                if(xmlReader.name() == "Processor") {
                                    xmlReader.readNext();
                                    diag.Processor = xmlReader.text().toString();
                                }
                                if(xmlReader.name() == "Memory") {
                                    xmlReader.readNext();
                                    diag.Memory = xmlReader.text().toString();
                                }
                                if(xmlReader.name() == "DirectXVersion") {
                                    xmlReader.readNext();
                                    diag.DirectXVersion = xmlReader.text().toString();
                                }
                            }
                            xmlReader.readNext();
                        }
                    }
                    if (xmlReader.name() == "DisplayDevice") {
                        if (xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() == "DisplayDevice")
                            break;

                        xmlReader.readNext();
                        while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "DisplayDevice"))
                        {
                            if (xmlReader.tokenType() == QXmlStreamReader::StartElement) {
                                if(xmlReader.name() == "CardName") {
                                    xmlReader.readNext();
                                    diag.CardName = xmlReader.text().toString();
                                }
                                if(xmlReader.name() == "SharedMemory") {
                                    xmlReader.readNext();
                                    diag.SharedMemory = xmlReader.text().toString();
                                }
                            }
                            xmlReader.readNext();
                        }
                    }
                    break;
                }
                default: break;
            }
        }
    }
    return diag;
}

// Получаем информацию из ХМЛки Yoma config mods
QStringList XMLParser::getMods() {

    qInfo() << "XMLParser::getMods: xml parse start";

    QStringList Mods;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qWarning() << "XMLParser::getMods: Parse error. " << xmlReader.errorString() << xmlReader.text();
            return Mods;
        } else {
            switch (xmlReader.tokenType()) {
                case QXmlStreamReader::StartDocument:
                    continue;
                case QXmlStreamReader::StartElement: {
                    if (xmlReader.name() == "DSServer")
                        continue;
                    if (xmlReader.name() == "Mods") {
                        QString element;
                        if (xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() == "Mods")
                            break;

                        xmlReader.readNext();
                        while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "Mods"))
                        {
                            if (xmlReader.tokenType() == QXmlStreamReader::StartElement) {
                                if(xmlReader.name() == "Name") {
                                    xmlReader.readNext();
                                    element = xmlReader.text().toString();
                                }
                            }
                            xmlReader.readNext();
                        }
                        Mods.append(element);
                    }
                    break;
                }
                default: break;
            }
        }
    }
    return Mods;
}

// Парсим XMLку Addon'ов
QList< QMap<QString, QString> > XMLParser::getAddons() {

    qInfo() << "XMLParser::getAddons: xml parse start";

    QList< QMap<QString, QString> > Addons;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qWarning() << "XMLParser::getAddons: Parse error. " << xmlReader.errorString() << xmlReader.text();
            return Addons;
        } else {
            switch (xmlReader.tokenType()) {
                case QXmlStreamReader::StartDocument:
                    continue;
                case QXmlStreamReader::StartElement: {
                    if (xmlReader.name() == "DSServer")
                        continue;
                    if (xmlReader.name() == "Addons") {
                        QMap<QString, QString> element;
                        if (xmlReader.tokenType() != QXmlStreamReader::StartElement && xmlReader.name() == "Addons")
                            break;

                        xmlReader.readNext();
                        while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "Addons"))
                        {
                            if (xmlReader.tokenType() == QXmlStreamReader::StartElement) {
                                if(( xmlReader.name() == "Md5"  || xmlReader.name() == "Path" || xmlReader.name() == "Pbo"
                                  || xmlReader.name() == "Size" || xmlReader.name() == "Url"  )) {
                                    QString elementName = xmlReader.name().toString();
                                    xmlReader.readNext();
                                    element.insert(elementName, xmlReader.text().toString());
                                }
                            }
                            xmlReader.readNext();
                        }
                        Addons.append(element);
                    }
                    break;
                }
                default: break;
            }
        }
    }
    return Addons;
}
