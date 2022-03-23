#ifndef DEVICESUBTYPEWIDGET_H
#define DEVICESUBTYPEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
class DeviceSubTypeWidget;
}

class DeviceSubTypeWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit DeviceSubTypeWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~DeviceSubTypeWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::DeviceSubTypeWidget *ui;

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

#endif // DEVICESUBTYPEWIDGET_H
