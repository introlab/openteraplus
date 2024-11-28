#ifndef SURVEYSERVICECONFIGWIDGET_H
#define SURVEYSERVICECONFIGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QHash>

#include "managers/ComManager.h"
#include "SurveyComManager.h"
#include "data/Message.h"

namespace Ui {
class SurveyServiceConfigWidget;
}

class SurveyServiceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SurveyServiceConfigWidget(ComManager* comManager, int id_project, QWidget *parent = nullptr);
    ~SurveyServiceConfigWidget();

    void refresh();

private slots:
    void on_btnNew_clicked();
    void on_btnUndo_clicked();
    void on_btnSave_clicked();

    void processTestTypesReply(QList<TeraData> ttp_list, QUrlQuery reply_query);

    void processActiveSurveyReply(const QJsonObject survey);
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void surveyComManagerReady(bool ready);

    void nextMessageWasShown(Message current_message);

    void on_txtName_textChanged(const QString &arg1);

    void on_lstData_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::SurveyServiceConfigWidget *ui;

    ComManager*         m_comManager;
    SurveyComManager*   m_surveyComManager;

    int             m_id_project;
    int             m_id_survey = 0;
    TeraData*       m_data = nullptr;

    bool        m_refreshRequested = false;

    QHash<QString, QListWidgetItem*>    m_listTestTypes_items; // Test type UUID mapping

    void connectSignals();
    void queryTestTypes();
    void setEditMode(const bool& editing);


};

#endif // SURVEYSERVICECONFIGWIDGET_H
