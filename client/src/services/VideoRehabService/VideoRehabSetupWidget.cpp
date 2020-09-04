#include "VideoRehabSetupWidget.h"
#include "ui_VideoRehabSetupWidget.h"

VideoRehabSetupWidget::VideoRehabSetupWidget(ComManager *comManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoRehabSetupWidget),
    m_comManager(comManager)
{
    ui->setupUi(this);

    initUI();
}

VideoRehabSetupWidget::~VideoRehabSetupWidget()
{
    delete ui;
}

void VideoRehabSetupWidget::initUI()
{
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);

    //Set page to view
    m_webEngine->setPage(m_webPage);


    // Create layout for widget if missing
    if (!ui->wdgWebEngine->layout()){
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgWebEngine->setLayout(layout);
    }
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_webEngine->setSizePolicy(sizePolicy);
    ui->wdgWebEngine->layout()->addWidget(m_webEngine);

    m_webEngine->setUrl(QUrl("https://www.google.ca"));
}
