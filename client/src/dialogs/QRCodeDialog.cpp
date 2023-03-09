#include "QRCodeDialog.h"
#include "ui_QRCodeDialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QStandardPaths>

#include "Utils.h"

QRCodeDialog::QRCodeDialog(QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QRCodeDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    ui->wdgCode->setFixedSize(this->height() - 80, this->height() - 80);

    connect(ui->btnClose, &QPushButton::clicked, this, &QDialog::accept);

    setText(text);

}


QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}

void QRCodeDialog::setText(const QString &text)
{
    if (!text.isEmpty()){
        ui->wdgCode->setText(text);
    }
}

void QRCodeDialog::setContext(const QString &context)
{
    m_context = context;
}

void QRCodeDialog::on_btnCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(ui->wdgCode->grab());
}


void QRCodeDialog::on_btnSave_clicked()
{
    QStringList suggested_paths = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QString base_path;
    if (!suggested_paths.isEmpty())
        base_path = suggested_paths.first();

    QString default_name = Utils::removeNonAlphanumerics(Utils::removeAccents(m_context));
    QString save_file = QFileDialog::getSaveFileName(this, tr("Enregistrer le code QR"),  base_path + "/" + default_name, tr("Images") + " (*.png)");
    if (!save_file.isEmpty()){
        ui->wdgCode->grab().save(save_file, "PNG");
    }
}

