#include "DanceConfigWidget.h"
#include "ui_DanceConfigWidget.h"

DanceConfigWidget::DanceConfigWidget(ComManager *comManager, int projectId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DanceConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId)
{
    ui->setupUi(this);

    m_danceComManager = new DanceComManager(comManager);
    m_uploadDialog = nullptr;

    ui->tabMain->setCurrentIndex(0);

    connectSignals();

}

DanceConfigWidget::~DanceConfigWidget()
{
    delete ui;
    delete m_danceComManager;

    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }
}

void DanceConfigWidget::on_btnUpload_clicked()
{
    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    m_uploadDialog = new FileUploaderDialog(dynamic_cast<QWidget*>(this));

    connect(m_uploadDialog, &FileUploaderDialog::finished, this, &DanceConfigWidget::fileUploaderFinished);

    m_uploadDialog->setModal(true);
    m_uploadDialog->show();

}

void DanceConfigWidget::processVideosReply(QList<QJsonObject> videos)
{
    ui->lstVideos->clear();

    for (const QJsonObject &video:qAsConst(videos)){
        QString video_name = video["video_label"].toString();
        int id_video = video["id_video"].toInt();
        if (!video_name.isEmpty()){
            QListWidgetItem* item = new QListWidgetItem();
            item->setIcon(QIcon("://icons/data_video.png"));
            item->setText(video_name);
            item->setData(Qt::UserRole, id_video);

            ui->lstVideos->addItem(item);
        }

    }
}

void DanceConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(op)
    Q_UNUSED(error)

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    GlobalMessageBox msg;
    msg.showError(tr("Une erreur est survenue..."), error_msg);

}

void DanceConfigWidget::fileUploaderFinished(int result)
{
    if (result == QDialog::Accepted){
        // Do the upload process!
        QStringList files = m_uploadDialog->getFiles();
        QStringList labels = m_uploadDialog->getLabels();
    }

    m_uploadDialog->deleteLater();
    m_uploadDialog = nullptr;
}

void DanceConfigWidget::connectSignals()
{
    connect(m_danceComManager, &DanceComManager::videosReceived, this, &DanceConfigWidget::processVideosReply);
    connect(m_danceComManager, &DanceComManager::networkError, this, &DanceConfigWidget::handleNetworkError);
}


void DanceConfigWidget::on_tabMain_currentChanged(int index)
{
    QWidget* current_tab = ui->tabMain->widget(index);

    if (current_tab == ui->tabVideos){
        // Refresh videos in library
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
        m_danceComManager->doGet(DANCE_VIDEO_PATH, query);
    }

}

