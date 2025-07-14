#ifndef MOVECONFIGWIDGET_H
#define MOVECONFIGWIDGET_H

#include <QWidget>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QStyledItemDelegate>

#include "managers/ComManager.h"
#include "MOVEComManager.h"

namespace Ui {
class MOVEConfigWidget;
}

class MOVEConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MOVEConfigWidget(ComManager* comManager, int projectId, QString participantUuid = QString(), QWidget *parent = nullptr);
    ~MOVEConfigWidget();

private:
    Ui::MOVEConfigWidget   *ui;


    ComManager* m_comManager;
    int         m_idProject;
    QString     m_uuidParticipant;
    MOVEComManager* m_MOVEComManager;

    void connectSignals();
};

#endif // MOVECONFIGWIDGET_H
