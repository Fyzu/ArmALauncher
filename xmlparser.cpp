#include "xmlparser.h"

XMLParser::XMLParser(QString XMLPath) {

    QFile file(XMLPath);
    if(file.open(QIODevice::ReadOnly)) {
        xmlReader.addData(file.readAll());
    } else {
        qDebug() << "Debug-fileRepo: XML Open Error - file: " << file.fileName();
    }

}

XMLParser::~XMLParser() {

}

void XMLParser::setPath(QString path) {

    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        xmlReader.clear();
        xmlReader.addData(file.readAll());
        file.close();
    } else {
        qDebug() << "Debug-fileRepo: XML Open Error - file: " << file.fileName();
    }

}

// Получить
dxdiag XMLParser::getDxdiag() {
    dxdiag diag;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qDebug() << "Debug-fileRepo: Parse error. " << xmlReader.errorString() << xmlReader.text();
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

//
QStringList XMLParser::getMods() {

    QStringList Mods;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qDebug() << "Debug-fileRepo: Parse error. " << xmlReader.errorString() << xmlReader.text();
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

    QList< QMap<QString, QString> > Addons;

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.hasError()) {
            qDebug() << "Debug-fileRepo: Parse error. " << xmlReader.errorString() << xmlReader.text();
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

