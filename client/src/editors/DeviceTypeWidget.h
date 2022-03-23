#ifndef DEVICETYPEWIDGET_H
#define DEVICETYPEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
    class DeviceTypeWidget;
}

class DeviceTypeWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit DeviceTypeWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~DeviceTypeWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::DeviceTypeWidget *ui;

    QMap<int, QListWidgetItem*>  m_listDevices_items;

    void updateControlsState();
    void updateFieldsValue();

    void updateDevice(const TeraData* device);

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processDevicesReply(QList<TeraData> devices);

    void postResultReply(QString path);

    void on_tabNav_currentChanged(int index);
};

#endif // DEVICETYPEWIDGET_H
