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

    void updateControlsState();
    void updateFieldsValue();

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void postResultReply(QString path);

    void btnSave_clicked();
    void btnUndo_clicked();

};

#endif // DEVICESUBTYPEWIDGET_H
