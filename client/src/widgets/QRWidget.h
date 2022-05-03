#ifndef QRWIDGET_H
#define QRWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>

#include "qrcodegen.hpp"

class QRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRWidget(QWidget *parent = nullptr);
    ~QRWidget();

    void setText(const QString &text);
    void setForegroundColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *paint_event) override;

    void paintQR(QPainter &painter);

    QString m_text;
    QColor m_foregroundColor;
};

#endif // QRWIDGET_H
