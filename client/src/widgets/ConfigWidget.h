#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>

#include <QListWidgetItem>
#include <QHBoxLayout>

#include "managers/ComManager.h"
#include "data/IconMenuDelegate.h"
#include "TeraData.h"

#include "GlobalMessageBox.h"

#include "editors/DataListWidget.h"

namespace Ui {
class ConfigWidget;
}

class ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWidget(ComManager* comMan, QWidget *parent = nullptr);
    ~ConfigWidget();

private slots:

    void currentSectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void com_Waiting(bool waiting);
    void com_NetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op);

    void on_tabSectionsWidget_currentChanged(int index);

private:

    Ui::ConfigWidget *ui;

    ComManager*         m_comManager;
    DataListWidget*     m_dataListEditor;

    void connectSignals();

    void addSection(const QString& name, const QIcon& icon, const int& id);
    void setupSections();

signals:
    void closeRequest();
};

#endif // CONFIGWIDGET_H
