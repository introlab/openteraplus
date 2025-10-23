#ifndef ACTIMETRYCONFIGWIDGET_H
#define ACTIMETRYCONFIGWIDGET_H

#include <QWidget>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QMap>
#include <QJsonObject>

#include "managers/ComManager.h"
#include "data/Message.h"
#include "ActimetryComManager.h"

namespace Ui
{
    class ActimetryConfigWidget;
}

class ActimetryConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ActimetryConfigWidget(ComManager *comManager, int projectId, QString participantUuid = QString(),
        QString participantName = QString(), QWidget *parent = nullptr);
    ~ActimetryConfigWidget();

public slots:
    void serviceReadyChanged(bool ready);
    void nextMessageWasShown(Message current_message);
    void availableAlgorithmsReceived(const QList<QJsonObject> &algorithms);
    void algorithmInfoReceived(const QJsonObject &algorithmInfo);
    void onComboBoxAlgorithmCurrentIndexChanged(int index);
    void onRunButtonClicked();

private:
    Ui::ActimetryConfigWidget *m_ui;

    ComManager *m_comManager;
    int m_idProject;
    QString m_participantUuid;
    QString m_participantName;
    ActimetryComManager *m_ActimetryComManager;

    QMap<QString, QJsonObject> m_algorithmInformations;


    void connectSignals();
    void setCurrentAlgorithmParametersInUI(const QString &algorithmKey);

private slots:
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
};

#endif // ACTIMETRYCONFIGWIDGET_H
