#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>

#include <QVariantList>
#include <QVariantMap>

#include <QComboBox>

#include "DataEditorWidget.h"

namespace Ui {
class UserWidget;
}

class UserWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit UserWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~UserWidget();

    void setData(const TeraData* data) override;

    void saveData(bool signal=true) override;

    void connectSignals();

    //void processQueryReply(const QString &path, const QUrlQuery &query_args, const QString &data);
    //void processPostReply(const QString &path, const QString &data);

private:
    Ui::UserWidget* ui;

    QHash<int, QListWidgetItem*>    m_listUserGroups_items;
    QHash<int, QListWidgetItem*>    m_listUserUserGroups_items;
    QHash<int, QString>             m_userSitesRole;

    void initUI();

    bool m_currentUserPasswordChanged;
    bool m_passwordJustGenerated;

    void updateControlsState() override;
    void updateFieldsValue() override;

    bool validateData() override;

    void refreshUsersUserGroups();

    QJsonArray getSelectedGroupsAsJsonArray();

    void buildUserPreferencesWidget();

    void updateUserGroup(const TeraData* group);
    void updateSiteAccess(const TeraData* site_access);
    void updateProjectAccess(const TeraData* project_access);
    void updateServiceAccess(const TeraData* service_access);

    void queryUserAccess();

    bool validateUserGroups();

public slots:


private slots:
    void processUsersReply(QList<TeraData> users);
    void processSitesAccessReply(QList<TeraData> sites);
    void processProjectsAccessReply(QList<TeraData> projects);
    void processServicesAccessReply(QList<TeraData> services_access);
    void processUserGroupsReply(QList<TeraData> user_groups, QUrlQuery query);
    void processUserUsersGroupsReply(QList<TeraData> user_users_groups, QUrlQuery query);
    void processUserPrefsReply(QList<TeraData> user_prefs, QUrlQuery query);
    void processFormsReply(QString form_type, QString data);
    void postResultReply(QString path);

    void on_tabMain_currentChanged(int index);
    void on_btnUpdateGroups_clicked();

    void userFormValueChanged(QWidget* widget, QVariant value);
    void userFormValueHasFocus(QWidget* widget);
    void on_btnUpdatePrefs_clicked();

    void editToggleClicked() override;
    void saveButtonClicked() override;
    void undoButtonClicked() override;

    void on_btnReset2FA_clicked();
};



#endif // USERWIDGET_H
