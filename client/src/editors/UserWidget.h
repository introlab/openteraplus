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
#include "GlobalMessageBox.h"

namespace Ui {
class UserWidget;
}

class UserWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit UserWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~UserWidget();

    void setData(const TeraData* data);

    void saveData(bool signal=true);

    void connectSignals();

    //void processQueryReply(const QString &path, const QUrlQuery &query_args, const QString &data);
    //void processPostReply(const QString &path, const QString &data);

private:
    Ui::UserWidget* ui;

    QMap<int, QListWidgetItem*>     m_listUserGroups_items;

    void updateControlsState();
    void updateFieldsValue();

    bool validateData();

    void refreshUsersUserGroups();


    void updateUserGroup(const TeraData* group);
    void updateSiteAccess(const TeraData* site_access);
    void updateProjectAccess(const TeraData* project_access);

public slots:


private slots:
    void processUsersReply(QList<TeraData> users);
    void processSitesAccessReply(QList<TeraData> sites);
    void processProjectsAccessReply(QList<TeraData> projects);
    void processUserGroupsReply(QList<TeraData> user_groups, QUrlQuery query);
    void processFormsReply(QString form_type, QString data);
    void postResultReply(QString path);

    void on_tabMain_currentChanged(int index);
    void on_btnUpdateGroups_clicked();
};



#endif // USERWIDGET_H
