#ifndef MOVECONFIGWIDGET_H
#define MOVECONFIGWIDGET_H

#include <QWidget>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QStyledItemDelegate>

#include "managers/ComManager.h"
#include "data/Message.h"
#include "MOVEComManager.h"

namespace Ui
{
    class MOVEConfigWidget;
}

class MOVEConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MOVEConfigWidget(ComManager *comManager, int projectId, QString participantUuid = QString(), 
        QString participantName = QString(), QWidget *parent = nullptr);
    ~MOVEConfigWidget();

public slots:

    void participantProfileReceived(QJsonObject user_profile, QUrlQuery reply_query);
    void participantProfileNotFound(const QString &path, int status_code, const QString &error_str);
    void serviceReadyChanged(bool ready);
    void onSaveParticipantProfileButtonClicked();
    void nextMessageWasShown(Message current_message);

private:

    void updateProfile(const QJsonObject &profile_data);

    Ui::MOVEConfigWidget *m_ui;

    ComManager *m_comManager;
    int m_idProject;
    QString m_uuidParticipant;
    QString m_participantName;
    QString m_participantLocation;
    MOVEComManager *m_MOVEComManager;
    QJsonObject m_participantProfile;

    void connectSignals();
};

#endif // MOVECONFIGWIDGET_H
