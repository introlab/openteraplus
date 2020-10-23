#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QUrl server_url, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Initialize web engine view
    m_webEngine = new QWebEngineView(ui->wdgWebView);
    m_webPage = new AboutDialogPage();
    m_webEngine->setPage(m_webPage);

    //Load about page
    QString url = server_url.toString() + "/about";
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
