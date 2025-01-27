#ifndef DESKTOPSHAREDIALOG_H
#define DESKTOPSHAREDIALOG_H

#include <QDialog>
#include <QAbstractListModel>

namespace Ui {
class DesktopShareDialog;
}

class DesktopShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DesktopShareDialog(QWidget *parent = nullptr);
    ~DesktopShareDialog();

    void setScreens(QAbstractListModel* screens);
    void setWindows(QAbstractListModel* windows);

    int getSelectedScreen();
    int getSelectedWindow();

private slots:
    void on_btnCancel_clicked();

    void on_btnShareScreen_clicked();

    void on_btnShareWindow_clicked();

private:
    Ui::DesktopShareDialog *ui;

    int m_selectedScreen = -1;
    int m_selectedWindow = -1;
};

#endif // DESKTOPSHAREDIALOG_H
