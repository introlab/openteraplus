#ifndef INSESSIONWIDGET_H
#define INSESSIONWIDGET_H

#include <QWidget>
#include "ComManager.h"
#include "TeraData.h"
#include "GlobalMessageBox.h"
#include "StartSessionDialog.h"
#include "Logger.h"

#include "services/BaseServiceWidget.h"
#include "services/VideoRehabService/VideoRehabWidget.h"

#include "JoinSessionReply.pb.h"

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
    void setSessionId(int session_id);



private slots:
    void on_btnEndSession_clicked();
    void on_btnInSessionInfos_toggled(bool checked);

    void processSessionsReply(QList<TeraData> sessions);

    void processJoinSessionEvent(JoinSessionEvent event);

    void showEvent(QShowEvent *event) override;

    void showStartSessionDiag(const QString& msg);
    void startSessionDiagTimeout();
    void startSessionDiagClosed();

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
    StartSessionDialog* m_startDiag;
};

#endif // INSESSIONWIDGET_H
