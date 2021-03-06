#ifndef SESSIONTYPEWIDGET_H
#define SESSIONTYPEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

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

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QListWidgetItem*>  m_listTypesProjects_items;

    void updateControlsState();
    void updateFieldsValue();

    void updateProject(TeraData* project);

    bool validateData();
    bool validateProjects();

    QJsonArray getSelectedProjectsAsJsonArray();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionTypesProjectsReply(QList<TeraData> stp_list);
    void processProjectsReply(QList<TeraData> projects);
    void postResultReply(QString path);

    void btnSaveProjects_clicked();

    void on_lstProjects_itemChanged(QListWidgetItem *item);
    void on_tabNav_currentChanged(int index);
    void on_btnUpdateConfig_clicked();
};

#endif // SESSIONTYPEWIDGET_H
