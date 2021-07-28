#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ComManager.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:
    void connectClicked();
    void newLogEntry(int type, QString message, QString sender);
private:
    Ui::MainWindow *ui;
    ComManager *m_comManager;

};
#endif // MAINWINDOW_H
