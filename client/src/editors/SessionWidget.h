#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

#include "TeraSessionType.h"
#include "DataEditorWidget.h"

namespace Ui {
class SessionWidget;
}

class SessionWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit SessionWidget(ComManager* comMan, const TeraData* data, QWidget *parent = nullptr);
    ~SessionWidget();

    void saveData(bool signal=true);
    void setData(const TeraData *data);

    void alwaysShowAssets(const bool &allow);
    void showAssets();
    void showTests();

    void setTestTypes(QList<TeraData>* test_types);
    void enableEmails(const bool& enable);

    void connectSignals();
private:
    Ui::SessionWidget *ui;

    int     m_idProject;
    QString m_baseParticipantUuid;  // Base participant UUID when editing a new session
    bool    m_alwaysShowAssets;       // Allow creation of new assets, even when no assets are present
    bool    m_allowEmails = false;

    //QMap<int, QTableWidgetItem*> m_listDeviceDatas;
    QMap<int, QTableWidgetItem*> m_listSessionEvents;
    QList<TeraData>*    m_testTypes = nullptr;
    TeraSessionType     m_sessionType;

    void updateControlsState();
    void updateFieldsValue();

    bool validateData();

    void updateSessionParticipants();
    void updateSessionUsers();
    void updateSessionDevices();
    //void updateDeviceData(TeraData* device_data);
    void updateEvent(TeraData* event);

    void queryAvailableInvitees();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionEventsReply(QList<TeraData> events);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);
    void processParticipantsReply(QList<TeraData> participants);
    void processDevicesReply(QList<TeraData> devices);
    void processUsersReply(QList<TeraData> users);
    void processSessionTypesReply(QList<TeraData> sessions_types, QUrlQuery query);
    void postResultReply(QString path);
    void deleteDataReply(QString path, int id);

    void onAssetsCountChanged(int new_count);
    void onTestsCountChanged(int new_count);

    void sessionInviteesChanged(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);

    void on_icoUsers_clicked();
    void on_icoParticipant_clicked();
    void on_icoDevices_clicked();
    void on_icoEvents_clicked();
    void on_icoAssets_clicked();
    void on_icoTests_clicked();

    void on_tabNav_currentChanged(int index);

    void on_btnEditInvitees_toggled(bool checked);

signals:
    void assetsCountChanged(int id_session, int new_count);
    void testsCountChanged(int id_session, int new_count);
};

#endif // SESSIONWIDGET_H
