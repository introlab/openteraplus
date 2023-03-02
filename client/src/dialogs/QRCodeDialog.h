#ifndef QRCODEDIALOG_H
#define QRCODEDIALOG_H

#include <QDialog>

namespace Ui {
class QRCodeDialog;
}

class QRCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRCodeDialog(QString text = QString(), QWidget *parent = nullptr);
    ~QRCodeDialog();

    void setText(const QString& text);
    void setContext(const QString& context);

private slots:
    void on_btnCopy_clicked();

    void on_btnSave_clicked();

private:
    Ui::QRCodeDialog *ui;

    QString m_context = "code";
};

#endif // QRCODEDIALOG_H
