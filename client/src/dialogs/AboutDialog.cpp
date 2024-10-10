#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(ComManager* comManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Initialize web engine view
    m_webEngine = new QWebEngineView(ui->wdgWebView);
    m_webEngine->setContextMenuPolicy(Qt::NoContextMenu);
    m_webPage = new AboutDialogPage();
    m_webEngine->setPage(m_webPage);

    m_comManager = comManager;

    //Load about page
    QString url = m_comManager->getServerUrl().toString() + "/about";
    m_webEngine->setUrl(url);

    // Create layout for widget if missing
    if (!ui->wdgWebView->layout()){
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgWebView->setLayout(layout);
    }
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_webEngine->setSizePolicy(sizePolicy);
    ui->wdgWebView->layout()->addWidget(m_webEngine);
    ui->wdgWebView->setFocus();

    // Request server settings
    connectSignals();

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_DEVICE_REGISTER_KEY, "1");
    m_comManager->doGet(WEB_SERVERSETTINGS_PATH, args);
}

AboutDialog::~AboutDialog()
{
    m_webEngine->deleteLater();
    m_webPage->deleteLater();
    delete ui;
}

void AboutDialog::on_btnOk_clicked()
{
    accept();
}

void AboutDialog::on_lblAbout_clicked()
{
    if (m_webEngine){
        m_webEngine->setUrl(QUrl("chrome://dino"));
    }
}

void AboutDialog::onPageLoaded()
{
    ui->wdgWebView->setFocus();
}

void AboutDialog::processServerSettings(QVariantHash settings)
{
    if (settings.contains(WEB_QUERY_DEVICE_REGISTER_KEY)){
        ui->lblDeviceRegisterKeyValue->setText(settings[WEB_QUERY_DEVICE_REGISTER_KEY].toString());
    }else{
        ui->lblDeviceRegisterKeyValue->setText(tr("Inconnue"));
    }
}

void AboutDialog::connectSignals()
{
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &AboutDialog::onPageLoaded);
    connect(m_comManager, &ComManager::serverSettingsReceived, this, &AboutDialog::processServerSettings);
}
