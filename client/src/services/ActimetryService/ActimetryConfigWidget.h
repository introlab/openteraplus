#ifndef ACTIMETRYCONFIGWIDGET_H
#define ACTIMETRYCONFIGWIDGET_H

#include <QWidget>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QStyledItemDelegate>

#include "managers/ComManager.h"
#include "data/Message.h"
#include "ActimetryComManager.h"

namespace Ui
{
    class ActimetryConfigWidget;
}

class ActimetryConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ActimetryConfigWidget(ComManager *comManager, int projectId, QString participantUuid = QString(),
        QString participantName = QString(), QWidget *parent = nullptr);
    ~ActimetryConfigWidget();

public slots:


    void serviceReadyChanged(bool ready);
    void nextMessageWasShown(Message current_message);

private:



    Ui::ActimetryConfigWidget *m_ui;

    ComManager *m_comManager;
    int m_idProject;
    ActimetryComManager *m_ActimetryComManager;


    void connectSignals();
};

#endif // ACTIMETRYCONFIGWIDGET_H
