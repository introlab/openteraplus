#ifndef SERVICEWIDGET_H
#define SERVICEWIDGET_H

#include <QWidget>

#include <QTableWidgetItem>
#include <QInputDialog>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
class ServiceWidget;
}

class ServiceWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ServiceWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ServiceWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::ServiceWidget *ui;

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QListWidgetItem*>  m_listRoles_items;
    QMap<int, QString>           m_serviceRoles;

    void updateControlsState();
    void updateFieldsValue();

    void updateServiceProject(TeraData* project);
    void updateServiceRole(const int &id_role, const QString &role_name);

    void postServiceRoles();

    void setData(const TeraData *data);

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void postResultReply(QString path);
    void processServiceProjects(QList<TeraData> projects);

    void on_tabNav_currentChanged(int index);
    void on_btnUpdateProjects_clicked();

    void on_lstProjects_itemChanged(QListWidgetItem *item);
    void on_lstRoles_currentRowChanged(int currentRow);
    void on_btnNewRole_clicked();
    void on_btnDeleteRole_clicked();
    void on_btnEditRole_clicked();
};

#endif // SERVICEWIDGET_H
