#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel {

Q_OBJECT

public:

    explicit ClickableLabel(QWidget* parent = nullptr);
    ~ClickableLabel();

signals:

    void clicked();

protected:

    void mouseReleaseEvent(QMouseEvent* event);

};

#endif // CLICKABLELABEL_H
