#ifndef TESTTYPEWIDGET_H
#define TESTTYPEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
class TestTypeWidget;
}

class TestTypeWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit TestTypeWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~TestTypeWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::TestTypeWidget *ui;

    QMap<int, QTreeWidgetItem*>  m_treeSites_items;
    QMap<int, QTreeWidgetItem*>  m_treeProjects_items;

    void updateControlsState();
    void updateFieldsValue();

    void updateTestTypeSite(TeraData* sts);
    void updateTestTypeProject(TeraData* stp);
    void updateSite(TeraData *site);
    void updateProject(TeraData *project);

    void postTestTypeSites();
    void postTestTypeProjects();

    bool validateProjects();
    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processTestTypesProjectsReply(QList<TeraData> stp_list);
    void processTestTypesSitesReply(QList<TeraData> sts_list);
    void processSitesReply(QList<TeraData> sites);
    void processProjectsReply(QList<TeraData> projects);
    void postResultReply(QString path);

    void btnSaveProjects_clicked();

    void on_treeProjects_itemChanged(QTreeWidgetItem *item, int column);
    void on_tabNav_currentChanged(int index);

};

#endif // TESTTYPEWIDGET_H
