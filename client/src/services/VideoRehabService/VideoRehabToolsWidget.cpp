#include "VideoRehabToolsWidget.h"
#include "ui_VideoRehabToolsWidget.h"

#include "GlobalMessageBox.h"

VideoRehabToolsWidget::VideoRehabToolsWidget(ComManager* comMan, BaseServiceWidget* baseWidget, QWidget *parent) :
    BaseServiceToolsWidget(comMan, baseWidget, parent),
    ui(new Ui::VideoRehabToolsWidget)
{
    ui->setupUi(this);
    setupTools();
    m_isRecording = false;
    m_recordWarningShown = false;
}

VideoRehabToolsWidget::~VideoRehabToolsWidget()
{
    delete ui;
}

void VideoRehabToolsWidget::on_btnReconnect_clicked()
{
    dynamic_cast<VideoRehabWidget*>(m_baseWidget)->reload();

}

void VideoRehabToolsWidget::setupTools()
{
    // Recording features
    ui->frameRecord->hide();
    if (m_comManager->getCurrentSessionType()){
        QString session_type_config = m_comManager->getCurrentSessionType()->getFieldValue("session_type_config").toString();
        if (!session_type_config.isEmpty()){

            QJsonDocument session_type_config_json = QJsonDocument::fromJson(session_type_config.toUtf8());
            if (!session_type_config_json.isNull()){
                if (session_type_config_json.object().contains("session_recordable")){
                    bool enable_recording = session_type_config_json.object()["session_recordable"].toBool();
                    ui->frameRecord->setVisible(enable_recording);
                }
            }
        }
    }
}

void VideoRehabToolsWidget::on_btnRecord_clicked()
{
    if (m_isRecording){
        // Toggle button text and icon
        ui->btnRecord->setIcon(QIcon("://icons/record.png"));
        ui->btnRecord->setText(tr("Enregistrer"));

        // Stop recording
        m_isRecording = false;
        dynamic_cast<VideoRehabWidget*>(m_baseWidget)->stopRecording();

    }else{
        // Start recording
        if (!m_recordWarningShown){
            // Display important legal warning about recording sessions
            GlobalMessageBox msg_box;

            QString legal_txt = tr("Vous vous apprêtez à enregistrer localement une séance vidéo.") + "\n\n" +
                    tr("En acceptant de poursuivre avec cet enregistrement, vous acceptez la responsabilité professionnelle, légale et éthique en lien avec la conservation, la diffusion, l'utilisation et la confidentialité requise avec ce type de média.") + "\n\n" +
                    tr("Souhaitez-vous toujours activer l'enregistrement vidéo?");

            if (msg_box.showYesNo(tr("Confirmation requise"), legal_txt) != QMessageBox::Yes){
                return;
            }
            m_recordWarningShown = true;
        }

        // Toggle button text and icon
        ui->btnRecord->setIcon(QIcon("://icons/record_stop.png"));
        ui->btnRecord->setText(tr("Arrêter l'enregistrement"));

        // Start recording
        m_isRecording = true;
        dynamic_cast<VideoRehabWidget*>(m_baseWidget)->startRecording();
    }
}
