#include "ConfigWidget.h"
#include "ui_ConfigWidget.h"

ConfigWidget::ConfigWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),  
    ui(new Ui::ConfigWidget),
    m_comManager(comMan)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    m_dataListEditor = nullptr;

    // Set default layout for widget editor, if needed
    if (ui->wdgEditor->layout() == nullptr){
        ui->wdgEditor->setLayout(new QHBoxLayout);
    }

    setupSections();

    connectSignals();

    ui->lstSections->setCurrentRow(0);
    ui->tabSectionsWidget->setCurrentIndex(0);
    ui->lstSections->setItemDelegate(new IconMenuDelegate(ui->lstSections->gridSize().height(), this));

    // Logs
    ui->logsWdg->setComManager(comMan);
    ui->logsWdg->setViewMode(LogViewWidget::VIEW_LOGS_ALL);
    ui->loginsWdg->setComManager(comMan);
    ui->loginsWdg->setViewMode(LogViewWidget::VIEW_LOGINS_ALL);

}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::addSection(const QString &name, const QIcon &icon, const int &id)
{
    QListWidgetItem *tmp = new QListWidgetItem(ui->lstSections);
    tmp->setIcon(icon);

    tmp->setText(name);
    tmp->setTextAlignment(Qt::AlignCenter);
    tmp->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tmp->setToolTip(name);
    //tmp->setSizeHint(ui->lstSections->gridSize());
    tmp->setData(Qt::UserRole, id);
}

void ConfigWidget::setupSections()
{
    ui->lstSections->setGridSize(QSize(ui->lstSections->width()-20, ui->lstSections->gridSize().height()));
    addSection(tr("Utilisateurs"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)), TERADATA_USER);
    addSection(tr("Groupes utilisateurs"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_USERGROUP)), TERADATA_USERGROUP);
    addSection(tr("Sites"), QIcon("://icons/site-icon.png"), TERADATA_SITE);
    addSection(tr("Appareils"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)), TERADATA_DEVICE);
    addSection(tr("Type appareils"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICETYPE)), TERADATA_DEVICETYPE);
    addSection(tr("Sous-types appareils"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICESUBTYPE)), TERADATA_DEVICESUBTYPE);
    addSection(tr("Types de séances"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSIONTYPE)), TERADATA_SESSIONTYPE);
    addSection(tr("Types évaluations"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_TESTTYPE)), TERADATA_TESTTYPE);
    if (m_comManager->isCurrentUserSuperAdmin()){
        addSection(tr("Services"), QIcon(TeraData::getIconFilenameForDataType(TERADATA_SERVICE)), TERADATA_SERVICE);
    }

    //ui->lstSections->setItemSelected(ui->lstSections->item(0),true);
    //ui->lstSections->item(0)->setSelected(true);
}

void ConfigWidget::currentSectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    TeraDataTypes current_section = static_cast<TeraDataTypes>(current->data(Qt::UserRole).toInt());

    //qDebug() << current_section;

    if (m_dataListEditor){
        ui->wdgEditor->layout()->removeWidget(m_dataListEditor);
        m_dataListEditor->deleteLater();
    }

    m_dataListEditor = new DataListWidget(m_comManager, current_section);
    ui->wdgEditor->layout()->addWidget(m_dataListEditor);
    m_dataListEditor->setPermissions(true, true); // All permissions from here!

}

void ConfigWidget::com_Waiting(bool waiting)
{
    setDisabled(waiting);
}

void ConfigWidget::com_NetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op)
{
    Q_UNUSED(error)
    if (op != QNetworkAccessManager::DeleteOperation) { // Already managed in MainWindow!
        GlobalMessageBox error_diag(this);
        if (error_str.isEmpty() || error_str.startsWith("\"\""))
            error_str = tr("Erreur inconnue");
        error_diag.showError("Erreur", error_str);
    }
}

void ConfigWidget::connectSignals()
{
    connect(ui->lstSections, &QListWidget::currentItemChanged, this, &ConfigWidget::currentSectionChanged);

    connect(m_comManager, &ComManager::waitingForReply, this, &ConfigWidget::com_Waiting);
    connect(m_comManager, &ComManager::networkError, this, &ConfigWidget::com_NetworkError);

    //connect(ui->btnClose, &QPushButton::clicked, this, &ConfigWidget::closeRequest);
}

void ConfigWidget::on_tabSectionsWidget_currentChanged(int index)
{
    if (ui->tabSectionsWidget->currentWidget() == ui->tabLogs){
        ui->logsWdg->refreshData();
    }

    if (ui->tabSectionsWidget->currentWidget() == ui->tabLogins){
        ui->loginsWdg->refreshData();
    }
}

