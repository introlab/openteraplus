#ifndef SURVEYEDITORDIALOG_H
#define SURVEYEDITORDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QWebEngineCertificateError>
#include <QWebEngineLoadingInfo>

#include "SurveyComManager.h"

namespace Ui {
class SurveyEditorDialog;
}

class SurveyEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SurveyEditorDialog(SurveyComManager *surveyComManager, const QString &test_type_uuid, QWidget *parent = nullptr);
    ~SurveyEditorDialog();

    void loadEditor();

private slots:
    void on_btnClose_clicked();

    void onCertificateError(const QWebEngineCertificateError &certificateError);
    void onPageLoadingChanged(const QWebEngineLoadingInfo &loadingInfo);


private:
    Ui::SurveyEditorDialog  *ui;
    SurveyComManager        *m_surveyComManager;
    QWebEngineView          *m_webView = nullptr;
    QWebEnginePage          *m_webPage = nullptr;
    QString                 m_testTypeUuid;
};

#endif // SURVEYEDITORDIALOG_H
