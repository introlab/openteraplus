#ifndef DATALISTWIDGET_H
#define DATALISTWIDGET_H

#include <QWidget>
#include <QJsonDocument>
#include <QPushButton>
#include "DataEditorWidget.h"

#include "TeraData.h"

#include "ui_DataListWidget.h"

namespace Ui {
class DataListWidget;
}

class DataListWidget :  public QWidget
{
    Q_OBJECT

public:
    explicit DataListWidget(ComManager* comMan, TeraDataTypes data_type, QWidget *parent = nullptr);
    explicit DataListWidget(ComManager* comMan, TeraDataTypes data_type, QUrlQuery args, QWidget *parent = nullptr);
    explicit DataListWidget(ComManager* comMan, TeraDataTypes data_type, QString query_path, QUrlQuery args, QStringList extra_display_fields = QStringList(), QWidget *parent = nullptr);
    ~DataListWidget();

    void setPermissions(const bool& can_view, const bool& can_edit);
    void setFilterText(const QString& text);

private:
    Ui::DataListWidget*                 ui;
    DataEditorWidget*                   m_editor;
    QMap<TeraData*, QListWidgetItem*>   m_datamap;
    QMap<int, QString>                  m_extraInfos; // int = TeraData ID
    ComManager*                         m_comManager;
    TeraDataTypes                       m_dataType;
    QString                             m_queryPath;
    QStringList                         m_extraDisplayFields;

    bool                                m_copying;
    bool                                m_searching;
    bool                                m_newdata;

    // Permissions
    bool                                m_canEdit;
    bool                                m_canView;

    QUrlQuery                           m_queryArgs;

    void connectSignals();
    void queryDataList();
    void queryDataList(QUrlQuery args);

    void updateControlsState();

    TeraData *getCurrentData();
    QListWidgetItem *getItemForData(TeraData *data);
    void updateDataInList(TeraData *data, bool select_item=false);
    void deleteDataFromList(TeraData* data);
    void showEditor(TeraData *data);

    void setSearching(bool search);

    void clearDataList();

public slots:


private slots:
    void com_Waiting(bool waiting);

    void deleteDataReply(QString path, int id);
    void setDataList(QList<TeraData> list);

    void editor_dataDeleted();
    void editor_dataChanged();

    void searchChanged(QString new_search);
    void clearSearch();
    void lstData_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void newDataRequested();
    void deleteDataRequested();
    void copyDataRequested();
    void on_radBtnFilter_toggled(bool checked);
    void on_radBtnAll_toggled(bool checked);
};


#endif // DATALIST_WIDGET_H
