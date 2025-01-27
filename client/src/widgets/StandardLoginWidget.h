#ifndef STANDARDLOGINWIDGET_H
#define STANDARDLOGINWIDGET_H

#include <QWidget>
#include <QMovie>

namespace Ui {
class StandardLoginWidget;
}

class StandardLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StandardLoginWidget(QWidget *parent = nullptr);
    ~StandardLoginWidget();

    void setStatusMessage(const QString& message, const bool& error=false);
    void setFocus();

private:
    Ui::StandardLoginWidget *ui;
    QMovie*     m_animatedIcon;

signals:
    void standardLoginRequest(QString username, QString password);
    void quitRequest();

private slots:
    void on_btnConnect_clicked();
    void on_btnQuit_clicked();
};

#endif // STANDARDLOGINWIDGET_H
