#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>

#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

#include "widgets/TableDateWidgetItem.h"
#include "widgets/TableNumberWidgetItem.h"

namespace Ui {
class GroupWidget;
}

class GroupWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit GroupWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~GroupWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::GroupWidget *ui;

    QMap<int, QTableWidgetItem*> m_tableParticipants_items;

    void updateControlsState();
    void updateFieldsValue();

    //void updateParticipant(TeraData* participant);
    //void updateStats();
    void setData(const TeraData* data);

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);
    void postResultReply(QString path);
    //void processParticipants(QList<TeraData> participants);

};

#endif // GROUPWIDGET_H
