#include "launcher.h"
#include "ui_launcher.h"

// Отправка запроса сообщения
void launcher::on_sendButton_clicked() {
    qDebug()<< "launcher::on_sendButton_clicked: called";
    if(!chatConnected) {
        qWarning() << "launcher::on_sendButton_clicked: user not auth.";
        QMessageBox::warning(this, tr("Внимание!"), tr("Ошибка отправки сообщения.\nВы указали неверные данные для авторизации или нет соеденения."), QMessageBox::Ok);
        chatAuth();
        return;
    }

    QNetworkRequest req( QUrl( QString("http://bystolen.ru/tsgames_ex/chat.php?action=send") ) );
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QByteArray reqData;
    reqData.append("msg="+ui->msgLineEdit->text().toUtf8());
    senderChatMgr->post(req, reqData);
}

// Обработка запроса отправки сообщения
void launcher::finishSendMsgChat(QNetworkReply *reply) {
    qDebug()<< "launcher::finishSendMsgChat: called";
    if(reply) {
        if (reply->error() == QNetworkReply::NoError) {
            ui->msgLineEdit->clear();
            updateChat();
        } else {
            qDebug() << "launcher::finishSendMsgChat: " << "Failure";
        }
    }
}

// Запрос на авторизацию в чате
void launcher::chatAuth() {
    qDebug()<< "launcher::chatAuth: called";

    QNetworkRequest req( QUrl( QString("http://bystolen.ru/tsgames_ex/chat.php?action=reg") ) );
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QByteArray reqData;
    reqData.append("id="+settings.tushinoApiKey);
    authChatMgr->post(req, reqData);
}

// Обработка ответа авторизации
void launcher::finishChatAuth(QNetworkReply *reply) {
    qDebug()<< "launcher::finishChatAuth: called";
    if(reply) {
        if (reply->error() == QNetworkReply::NoError) {
            chatConnected = true;
        } else {
            chatConnected = false;
            qWarning() << "launcher::finishChatAuth: Failure";
        }
    }
}

// Запрос на обновлениие информации в чате
void launcher::updateChat() {
    qDebug()<< "launcher::updateChat: called";

    updateChatMgr->get(QNetworkRequest(QUrl(QString("http://bystolen.ru/tsgames_ex/chat.php?action=getL"))));
}

// Обработка запроса обновления информации в чате
void launcher::finishUpdateChat(QNetworkReply *reply) {
    qDebug()<< "launcher::finishUpdateChat: called";

    if (reply->error() == QNetworkReply::NoError) {
        ui->chatView->clear();
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonValue value = document.object().value("messages");
        QJsonArray array = value.toArray();
        foreach (const QJsonValue & v, array) {
            ui->chatView->addItem(v.toObject().value("name").toString()+ ": " + v.toObject().value("msg").toString());
        }
    }else {
        qDebug()<< "launcher::finishUpdateChat: error";
    }
}
