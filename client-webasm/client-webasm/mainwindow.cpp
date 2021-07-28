#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Logger::instance()->setRedirectionActivation(true);
    connect(Logger::instance(), &Logger::newLogEntry, this, &MainWindow::newLogEntry);
    ui->setupUi(this);

    m_comManager = nullptr;

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::connectClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectClicked()
{
    ui->textEdit->append("connectClicked");
    m_comManager = new ComManager(ui->serverLineEdit->text(), true, this);
    m_comManager->connectToServer(ui->usernameLineEdit->text(), ui->passwordLineEdit->text());
}

void MainWindow::newLogEntry(int type, QString message, QString sender)
{
    ui->textEdit->append(QString(type) + "[" + sender + "] " + message);
}

