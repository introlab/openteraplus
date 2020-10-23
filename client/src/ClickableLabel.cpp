#include "ClickableLabel.h"

ClickableLabel::ClickableLabel(QWidget* parent)
: QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);

}

ClickableLabel::~ClickableLabel() {

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    emit clicked();

}
