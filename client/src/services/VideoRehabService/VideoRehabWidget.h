#ifndef VIDEOREHABWIDGET_H
#define VIDEOREHABWIDGET_H

#include "services/BaseServiceWidget.h"
#include <QWebEngineView>
#include <QHBoxLayout>

namespace Ui {
class VideoRehabWidget;
}

class VideoRehabWidget : public BaseServiceWidget
{
    Q_OBJECT

public:
    explicit VideoRehabWidget(ComManager* comMan, QWidget *parent = nullptr);
    ~VideoRehabWidget();

    void initUI();

private slots:
    void on_txtURL_returnPressed();
    void webEngineURLChanged(QUrl url);

private:
    void connectSignals();

    Ui::VideoRehabWidget *ui;
    QWebEngineView* m_webEngine;
};

#endif // VIDEOREHABWIDGET_H
