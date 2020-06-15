#ifndef INSESSIONWIDGET_H
#define INSESSIONWIDGET_H

#include <QWidget>
#include "ComManager.h"
#include "TeraData.h"
#include "GlobalMessageBox.h"
#include "StartSessionDialog.h"

#include "services/BaseServiceWidget.h"
#include "services/VideoRehabService/VideoRehabWidget.h"

namespace Ui {
class InSessionWidget;
}

class InSessionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InSessionWidget(ComManager *comMan, const TeraData* session_type, const int id_session, QWidget *parent = nullptr);
    ~InSessionWidget();

    void disconnectSignals();

private slots:
    void on_btnEndSession_clicked();
    void on_btnInSessionInfos_toggled(bool checked);

    void processSessionsReply(QList<TeraData> sessions);

private:
    void connectSignals();
    void initUI();
    void updateUI();
    void setMainWidget(QWidget* wdg);
    TeraSessionCategory::SessionTypeCategories getSessionTypeCategory();

    Ui::InSessionWidget *ui;

    ComManager*         m_comManager;
    TeraData            m_sessionType;
    TeraData*           m_session;
    BaseServiceWidget*  m_serviceWidget;
};

#endif // INSESSIONWIDGET_H
