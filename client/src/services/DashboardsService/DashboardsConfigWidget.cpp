#include "DashboardsConfigWidget.h"
#include "ui_DashboardsConfigWidget.h"

DashboardsConfigWidget::DashboardsConfigWidget(ComManager *comManager, const int &id_site, const int &id_project, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardsConfigWidget)
{
    ui->setupUi(this);
    m_comManager = comManager;
    m_dashComManager = new DashboardsComManager(comManager);

    m_idSite = id_site;
    m_idProject = id_project;

    // Query dashboards
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "1");
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}

DashboardsConfigWidget::~DashboardsConfigWidget()
{
    delete ui;
    delete m_dashComManager;
}
