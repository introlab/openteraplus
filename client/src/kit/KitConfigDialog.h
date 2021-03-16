#ifndef KITCONFIGDIALOG_H
#define KITCONFIGDIALOG_H

#include <QDialog>
#include <QStyledItemDelegate>
#include <QListWidgetItem>

#include "managers/ComManager.h"

namespace Ui {
class KitConfigDialog;
}

class KitConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KitConfigDialog(ComManager* comMan, QWidget *parent = nullptr);
    ~KitConfigDialog();

private slots:
    void on_btnClose_clicked();

    void comManagerWaitingForReply(bool waiting);
    void comManagerNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);

    void processSitesReply(QList<TeraData> sites);
    void processProjectsReply(QList<TeraData> projects);
    void processGroupsReply(QList<TeraData> groups);
    void processParticipantsReply(QList<TeraData> participants,  QUrlQuery reply_query);

    void on_cmbSites_currentIndexChanged(int index);

    void on_cmbProjects_currentIndexChanged(int index);

    void on_cmbGroups_currentIndexChanged(int index);

    void on_lstParticipants_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_btnSetParticipant_clicked();

private:
    void initUi();
    void connectSignals();

    void setStatusMessage(QString msg, bool error = false);

    void querySites();
    void queryProjects(int id_site);
    void queryGroups(int id_project);
    void queryParticipantsForProject(int id_project);
    void queryParticipantsForGroup(int id_group);
    void queryParticipant(int id_participant);

    Ui::KitConfigDialog *ui;

    ComManager* m_comManager;
    bool        m_loading;
};

#endif // KITCONFIGDIALOG_H
