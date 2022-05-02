#include "QRWidget.h"

QRWidget::QRWidget(QWidget *parent) :
    QWidget(parent)
{
    m_text = "";
    m_foregroundColor = Qt::black;
}

QRWidget::~QRWidget()
{

}

void QRWidget::setText(const QString &text)
{
    m_text = text;
}

void QRWidget::setForegroundColor(const QColor &color)
{
    m_foregroundColor = color;
}

void QRWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    paintQR(painter);
}

void QRWidget::paintQR(QPainter &painter)
{
    QSize sz = rect().size();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(m_text.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    const int s=qr.getSize()>0?qr.getSize():1;
    const double w=sz.width();
    const double h=sz.height();
    const double aspect=w/h;
    const double size=((aspect>1.0)?h:w);
    const double scale=size/(s+2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_foregroundColor);
    for(int y=0; y<s; y++) {
        for(int x=0; x<s; x++) {
            const int color=qr.getModule(x, y);  // 0 for white, 1 for black
            if(0!=color) {
                const double rx1=(x+1)*scale, ry1=(y+1)*scale;
                QRectF r(rx1, ry1, scale, scale);
                painter.drawRects(&r,1);
            }
        }
    }
}
