#include "VideoRehabWidget.h"
#include "ui_VideoRehabWidget.h"

VideoRehabWidget::VideoRehabWidget(ComManager *comMan, QWidget *parent) :
    BaseServiceWidget(comMan, parent),
    ui(new Ui::VideoRehabWidget)
{
    ui->setupUi(this);

    initUI();
    connectSignals();

}

VideoRehabWidget::~VideoRehabWidget()
{
    m_webPage->deleteLater();
    m_webEngine->deleteLater();
    delete ui;

}

void VideoRehabWidget::initUI()
{
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);

    //Set page to view
    m_webEngine->setPage(m_webPage);

    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    QWebEngineProfile::defaultProfile()->setHttpCacheType(QWebEngineProfile::NoCache);


    // Create layout for widget if missing
    if (!ui->wdgWebEngine->layout()){
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgWebEngine->setLayout(layout);
    }
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_webEngine->setSizePolicy(sizePolicy);
    ui->wdgWebEngine->layout()->addWidget(m_webEngine);
}

bool VideoRehabWidget::handleJoinSessionEvent(const JoinSessionEvent &event)
{
    // Redirect web engine to session url
    if (m_webPage){
        m_webPage->setUrl(QString::fromStdString(event.session_url()));
    }

    return true; // Accepts the request

}

void VideoRehabWidget::on_txtURL_returnPressed()
{
    m_webEngine->setUrl(ui->txtURL->text());
}

void VideoRehabWidget::webEngineURLChanged(QUrl url)
{
    ui->txtURL->setText(url.toString());
}

void VideoRehabWidget::connectSignals()
{
    if (m_webEngine){
        //connect(m_webEngine, &QWebEngineView::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }
    if (m_webPage){
        connect(m_webPage, &QWebEnginePage::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }

}
