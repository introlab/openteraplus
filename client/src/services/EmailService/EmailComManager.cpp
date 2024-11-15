#include "EmailComManager.h"
#include "EmailServiceWebAPI.h"

EmailComManager::EmailComManager(ComManager *comManager, QObject *parent)
    : BaseComManager{comManager->getServerUrl(), parent},
    m_comManager(comManager)
{
    // Set initial user token
    setCredentials(m_comManager->getCurrentToken());

    // Connect signals
    connectSignals();
}

EmailComManager::~EmailComManager()
{

}

bool EmailComManager::processNetworkReply(QNetworkReply *reply)
{
    QString reply_path = reply->url().path();
    QString reply_data = reply->readAll();
    QUrlQuery reply_query = QUrlQuery(reply->url().query());
    //qDebug() << reply_path << " ---> " << reply_data << ": " << reply->url().query();

    bool handled = false;

    if (reply->operation()==QNetworkAccessManager::GetOperation){
        if (!handled){
            // General case
            handled = handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }
    }

    if (reply->operation()==QNetworkAccessManager::PostOperation){
        if (!handled){
            handled = handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit postResultsOK(reply_path, reply_data);
        }
    }

    if (reply->operation()==QNetworkAccessManager::DeleteOperation){
        // Extract id from url
        int id = 0;
        if (reply_query.hasQueryItem("id")){
            id = reply_query.queryItemValue("id").toInt();
        }
        emit deleteResultsOK(reply_path, id);
        handled=true;
    }

    return handled;
}

bool EmailComManager::handleDataReply(const QString &reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "DashboardsComManager::handleDataReply");
        return false;
    }

    // Browse each items received
    QList<QJsonObject> items;
    if (data_list.isArray()){
        QJsonArray data_list_array = data_list.array();
        for (const QJsonValue &data:std::as_const(data_list_array)){
            items.append(data.toObject());
        }
    }else{
        items.append(data_list.object());
    }

    // Check to emit correct signals for specific data types
    if (reply_path.endsWith(EMAIL_TEMPLATE_PATH)){
        if (!items.empty()){
            emit emailTemplateReceived(items.first());
        }
    }

    // Always emit generic signal
    emit dataReceived(items, reply_query);

    return true;
}

void EmailComManager::connectSignals()
{
    connect(m_comManager, &ComManager::userTokenUpdated, this, &EmailComManager::handleUserTokenUpdated);
}

void EmailComManager::handleUserTokenUpdated()
{
    // Update token
    setCredentials(m_comManager->getCurrentToken());
}
