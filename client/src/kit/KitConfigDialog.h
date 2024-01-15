#ifndef KITCONFIGDIALOG_H
#define KITCONFIGDIALOG_H

#include <QDialog>
#include <QStyledItemDelegate>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QStandardPaths>

#include "managers/ComManager.h"
#include "KitConfigManager.h"

#include "services/VideoRehabService/VideoRehabVirtualCamSetupDialog.h"
#include "services/VideoRehabService/VideoRehabPTZDialog.h"

#include "GlobalMessageBox.h"

namespace Ui {
class KitConfigDialog;
}

class KitConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KitConfigDialog(ComManager* comMan, KitConfigManager* kitConfig, QWidget *parent = nullptr);
    ~KitConfigDialog() override;

private slots:
    void on_btnClose_clicked();

    void comManagerWaitingForReply(bool waiting);
    void comManagerNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);

    void processSitesReply(QList<TeraData> sites);
    void processProjectsReply(QList<TeraData> projects);
    void processGroupsReply(QList<TeraData> groups);
    void processParticipantsReply(QList<TeraData> participants,  QUrlQuery reply_query);
    void processServicesReply(QList<TeraData> services);
    void processFormsReply(QString form_type, QString data);

    void configFormDirty(bool dirty);
    void configFormValueChanged(QWidget *wdg, QVariant value);

    void on_cmbSites_currentIndexChanged(int index);
    void on_cmbProjects_currentIndexChanged(int index);
    void on_cmbGroups_currentIndexChanged(int index);
    void on_lstParticipants_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_btnSetParticipant_clicked();
    void on_btnUnsetParticipant_clicked();
    void on_cmbServices_currentIndexChanged(int index);
    void on_btnSaveConfig_clicked();

    void on_chkTechSup_stateChanged(int arg1);

    void on_btnTechSup_clicked();

    void on_txtTechSup_textChanged(const QString &arg1);

    void on_chkOtherSoftware_stateChanged(int arg1);

    void on_btnOtherSoftware_clicked();

    void on_txtOtherSoftware_textChanged(const QString &arg1);

private:
    void initUi();
    void connectSignals();

    void updateSaveButtonState();

    void setStatusMessage(QString msg, bool error = false);

    void showPTZDialog();

    void querySites();
    void queryProjects(int id_site);
    void queryGroups(int id_project);
    void queryServices(int id_project);
    void queryParticipantsForProject(int id_project);
    void queryParticipantsForGroup(int id_group);
    void queryParticipant(int id_participant);
    void queryDeviceConfigForm();

    Ui::KitConfigDialog *ui;

    ComManager*             m_comManager;
    KitConfigManager*       m_kitConfig;
    bool                    m_loading;
    QHash<int, TeraData>    m_services;

    bool                    m_valueJustChanged;
};

#endif // KITCONFIGDIALOG_H
