#include "ScreenController.h"
#include <QDebug>

#ifdef Q_OS_WIN
    #include "Windows.h"
#endif


ScreenController::ScreenController(QScreen* target_screen, QObject *parent)
    : QObject{parent}
{
    m_screen = target_screen;

    // Initialize click
    m_clickTimer.setInterval(250);
    m_clickTimer.setSingleShot(true);
    connect(&m_clickTimer, &QTimer::timeout, this, &ScreenController::doMouseClickUp);
}

void ScreenController::doClick(int x, int y, int area_w, int area_h)
{
    // Execute a mouse click on that area
    #ifdef Q_OS_WIN
        QPoint target = getTargetPoint(x, y, area_w, area_h);

        SetCursorPos(target.x(), target.y());

        doMouseClickDown(true);
    #endif
}

void ScreenController::doMouseDown(int x, int y, int area_w, int area_h)
{
    #ifdef Q_OS_WIN
        QPoint target = getTargetPoint(x, y, area_w, area_h);

        SetCursorPos(target.x(), target.y());

        doMouseClickDown(false);
    #endif
}

void ScreenController::doMouseMove(int x, int y, int area_w, int area_h)
{
    #ifdef Q_OS_WIN
        QPoint target = getTargetPoint(x, y, area_w, area_h);

        SetCursorPos(target.x(), target.y());

    #endif
}

void ScreenController::doMouseUp(int x, int y, int area_w, int area_h)
{
    #ifdef Q_OS_WIN
        QPoint target = getTargetPoint(x, y, area_w, area_h);

        SetCursorPos(target.x(), target.y());

        doMouseClickUp();
    #endif
}

void ScreenController::doMouseClickDown(bool auto_click_up)
{
    // Simulate mouse click
#ifdef Q_OS_WIN
    tagINPUT input;
    input.type = INPUT_MOUSE;
    MOUSEINPUT mi;
    mi.mouseData = 0;
    mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input.mi = mi;
    SendInput(1, &input, sizeof(input));

    if (auto_click_up)
        m_clickTimer.start();
#endif
}

void ScreenController::doMouseClickUp()
{
    //qDebug() << "doMouseUp";
    // Simulate mouse click
#ifdef Q_OS_WIN
    tagINPUT input;
    input.type = INPUT_MOUSE;
    MOUSEINPUT mi;
    mi.mouseData = 0;
    mi.dwFlags = MOUSEEVENTF_LEFTUP;
    input.mi = mi;
    SendInput(1, &input, sizeof(input));
#endif
}

QPoint ScreenController::getTargetPoint(int x, int y, int area_w, int area_h)
{
    // Compute coordinates based on screen resolution and actual screen res
    int screen_h = m_screen->geometry().height() * m_screen->devicePixelRatio();
    int screen_w = m_screen->geometry().width() * m_screen->devicePixelRatio();
    int screen_offset_x = m_screen->geometry().x();
    int screen_offset_y = m_screen->geometry().y();

    int target_x = static_cast<int>(((float)x / (float)area_w) * (float)screen_w) + screen_offset_x;
    int target_y = static_cast<int>(((float)y / (float)area_h) * (float)screen_h) + screen_offset_y;

    //qDebug() << screen_w << " " << target_x << " - " << screen_h << " " << target_y << " - " << screen_offset_x << " " << screen_offset_y;

    return QPoint(target_x, target_y);
}
