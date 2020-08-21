#ifndef SERVICECONFIGWIDGET_H
#define SERVICECONFIGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
class ServiceConfigWidget;
}

class ServiceConfigWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ServiceConfigWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ServiceConfigWidget();

    void saveData(bool signal=true);

    void connectSignals();

    void setData(const TeraData* data);

private:
    Ui::ServiceConfigWidget *ui;

    void updateControlsState();
    void updateFieldsValue();

    bool validateData();

    bool m_gotFormStructure;

private slots:
    void processFormsReply(QString form_type, QString data);

    void postResultReply(QString path);

};

#endif // SERVICECONFIGWIDGET_H
