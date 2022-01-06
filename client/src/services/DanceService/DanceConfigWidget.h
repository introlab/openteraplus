#ifndef DANCECONFIGWIDGET_H
#define DANCECONFIGWIDGET_H

#include <QWidget>

namespace Ui {
class DanceConfigWidget;
}

class DanceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DanceConfigWidget(QWidget *parent = nullptr);
    ~DanceConfigWidget();

private slots:
    void on_btnUpload_clicked();

private:
    Ui::DanceConfigWidget *ui;
};

#endif // DANCECONFIGWIDGET_H
