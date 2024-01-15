#ifndef SERVICECONFIGWIDGET_H
#define SERVICECONFIGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "editors/DataEditorWidget.h"
#include "data/IconMenuDelegate.h"
#include "GlobalMessageBox.h"

#include "Utils.h"

namespace Ui {
class ServiceConfigWidget;
}

class ServiceConfigWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    // id_field_name: either id_user, id_device or id_participant, the field to which this service config should be associated
    explicit ServiceConfigWidget(ComManager* comMan, const QString id_field_name, const int id_field_value, QWidget *parent = nullptr);
    ~ServiceConfigWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::ServiceConfigWidget *ui;

    QString m_idFieldName;
    int     m_idFieldValue;
    int     m_currentIdService = -1;
    QMap<int, QListWidgetItem*>     m_listServices_items;
    bool    m_gotServiceForm;
    //QMap<int, TeraData>             m_servicesIdsData;

    QString m_specificId;

    void updateControlsState();
    void updateFieldsValue();

    void updateServiceConfig(const TeraData* config);
    void updateService(const TeraData* service);

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query);
    void processServicesReply(QList<TeraData> services, QUrlQuery query);

    void postResultReply(QString path);

    void on_lstServiceConfig_itemClicked(QListWidgetItem *item);
    void on_btnSave_clicked();
    void on_btnUndo_clicked();
    void on_cmbSpecific_currentIndexChanged(const int &current_index);
};

#endif // SERVICECONFIGWIDGET_H
