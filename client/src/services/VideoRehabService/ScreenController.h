#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include <QObject>
#include <QScreen>
#include <QTimer>

class ScreenController : public QObject
{
    Q_OBJECT
public:
    explicit ScreenController(QScreen *target_screen, QObject *parent = nullptr);

    void doClick(int x, int y, int area_w, int area_h);
    void doMouseDown(int x, int y, int area_w, int area_h);
    void doMouseMove(int x, int y, int area_w, int area_h);
    void doMouseUp(int x, int y, int area_w, int area_h);

private:
    QScreen*    m_screen;
    QTimer      m_clickTimer;

    void doMouseClickDown(bool auto_click_up);
    void doMouseClickUp();

    QPoint getTargetPoint(int x, int y, int area_w, int area_h);

signals:

};

#endif // SCREENCONTROLLER_H
