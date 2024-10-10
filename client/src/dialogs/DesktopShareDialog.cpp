#include "DesktopShareDialog.h"
#include "ui_DesktopShareDialog.h"

#include <QComboBox>

DesktopShareDialog::DesktopShareDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DesktopShareDialog)
{
    ui->setupUi(this);
}

DesktopShareDialog::~DesktopShareDialog()
{
    delete ui;
}

void DesktopShareDialog::setScreens(QAbstractListModel *screens)
{
    ui->cmbScreens->setModel(screens);
}

void DesktopShareDialog::setWindows(QAbstractListModel *windows)
{
    ui->cmbWindows->setModel(windows);
}

int DesktopShareDialog::getSelectedScreen()
{
    return m_selectedScreen;
}

int DesktopShareDialog::getSelectedWindow()
{
    return m_selectedWindow;
}

void DesktopShareDialog::on_btnCancel_clicked()
{
    reject();
}


void DesktopShareDialog::on_btnShareScreen_clicked()
{
    m_selectedWindow = -1;
    m_selectedScreen = ui->cmbScreens->currentIndex();
    accept();
}


void DesktopShareDialog::on_btnShareWindow_clicked()
{
    m_selectedScreen = -1;
    m_selectedWindow = ui->cmbWindows->currentIndex();
    accept();
}

