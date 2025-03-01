#ifndef SESSIONTYPEWIDGET_H
#define SESSIONTYPEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

#include "DataEditorWidget.h"

namespace Ui {
class SessionTypeWidget;
}

class SessionTypeWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit SessionTypeWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~SessionTypeWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::SessionTypeWidget *ui;

    QMap<int, QTreeWidgetItem*>  m_treeSites_items;
    QMap<int, QTreeWidgetItem*>  m_treeProjects_items;
    QMap<int, QListWidgetItem*>  m_lstServices_items;

    void updateControlsState();
    void updateFieldsValue();

    void updateSessionTypeSite(TeraData* sts);
    void updateSessionTypeProject(TeraData* stp);
    void updateSite(TeraData *site);
    void updateProject(TeraData *project);
    void updateSessionTypeService(TeraData *sts);

    void postSessionTypeSites();
    void postSessionTypeProjects();

    bool validateProjects();
    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionTypesProjectsReply(QList<TeraData> stp_list);
    void processSessionTypesSitesReply(QList<TeraData> sts_list);
    void processSitesReply(QList<TeraData> sites);
    void processProjectsReply(QList<TeraData> projects);
    void processSessionTypesServicesReply(QList<TeraData> services);
    void postResultReply(QString path);

    void btnSaveProjects_clicked();

    void on_treeProjects_itemChanged(QTreeWidgetItem *item, int column);
    void on_tabNav_currentChanged(int index);
    void on_btnUpdateConfig_clicked();
    void on_btnServices_clicked();
};

#endif // SESSIONTYPEWIDGET_H
