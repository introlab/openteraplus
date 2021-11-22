#ifndef PROJECTNAVIGATORTREE_H
#define PROJECTNAVIGATORTREE_H

#include <QTreeWidget>
#include <QDropEvent>
#include "ProjectNavigator.h"
#include "GlobalMessageBox.h"


class ProjectNavigatorTree : public QTreeWidget
{
    Q_OBJECT
public:
    ProjectNavigatorTree(QWidget *parent);
protected:
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

private:
    ProjectNavigator* m_projNav;

signals:
    void moveRequest(QTreeWidgetItem* src, QTreeWidgetItem* target);

};

#endif // PROJECTNAVIGATORTREE_H
