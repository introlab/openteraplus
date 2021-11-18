#ifndef CLEANUPDIALOG_H
#define CLEANUPDIALOG_H

#include <QDialog>
#include <QToolButton>

#include "managers/ComManager.h"
#include "widgets/TableDateWidgetItem.h"
#include "widgets/TableNumberWidgetItem.h"

#include "GlobalMessageBox.h"
#include "TeraData.h"


namespace Ui {
class CleanUpDialog;
}

class CleanUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CleanUpDialog(ComManager* comManager, QVariantList &data, TeraDataTypes data_type, QWidget *parent = nullptr);
    ~CleanUpDialog();

    void setFeatures(const bool &enable_delete, const bool &enable_disable);

private slots:
    void deleteResultOk(QString path, int id);
    void postResultOk(QString path);
    void comOperationError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);

    void on_btnOK_clicked();

    void btnDisable_clicked();
    void btnDelete_clicked();

    void on_btnDisableAll_clicked();

    void on_btnDeleteAll_clicked();

private:
    Ui::CleanUpDialog               *ui;
    ComManager*                     m_comManager;
    QVariantList                    m_data;
    QHash<QString, int>             m_columnsMap;
    QHash<QTableWidgetItem*, int>   m_itemsMap;
    TeraDataTypes                   m_dataType;

    QList<int>                      m_idsToManage;

    bool                            m_allowDelete;
    bool                            m_allowDisable;

    void connectSignals();

    void createTableColumns();
    void refreshData();

    void disableNextItem();
    void deleteNextItem();
};

#endif // CLEANUPDIALOG_H
