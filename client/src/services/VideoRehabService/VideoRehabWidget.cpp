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
    m_webEngine->deleteLater();
    delete ui;
}

void VideoRehabWidget::initUI()
{
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);

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
        connect(m_webEngine, &QWebEngineView::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }

}
