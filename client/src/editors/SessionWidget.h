#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

#include "TeraSessionStatus.h"
#include "TeraSessionEvent.h"
#include "DownloadedFile.h"
#include "dialogs/BaseDialog.h"

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

    void connectSignals();
private:
    Ui::SessionWidget *ui;

    int     m_idProject;
    QString m_baseParticipantUuid;  // Base participant UUID when editing a new session

    //QMap<int, QTableWidgetItem*> m_listDeviceDatas;
    QMap<int, QTableWidgetItem*> m_listSessionEvents;

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
    void btnDownload_clicked();
    void btnDeleteData_clicked();
    void btnDownloadAll_clicked();

    void processFormsReply(QString form_type, QString data);
    //void processDeviceDatasReply(QList<TeraData> device_datas);
    void processSessionEventsReply(QList<TeraData> events);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);
    void processParticipantsReply(QList<TeraData> participants);
    void processDevicesReply(QList<TeraData> devices);
    void processUsersReply(QList<TeraData> users);
    void postResultReply(QString path);
    void deleteDataReply(QString path, int id);

    void onDownloadCompleted(DownloadedFile* file);

    void sessionInviteesChanged(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);

    void currentSelectedDataChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void on_tabSessionInfos_tabBarClicked(int index);
    void on_icoUsers_clicked();
    void on_icoParticipant_clicked();
    void on_icoDevices_clicked();
    void on_icoEvents_clicked();
    void on_icoAssets_clicked();
    void on_icoTests_clicked();

};

#endif // SESSIONWIDGET_H
