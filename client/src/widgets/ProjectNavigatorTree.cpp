#include "ProjectNavigatorTree.h"

#include <QDebug>

ProjectNavigatorTree::ProjectNavigatorTree(QWidget *parent):
    QTreeWidget(parent)
{
    m_projNav = dynamic_cast<ProjectNavigator*>(parent);

}

void ProjectNavigatorTree::dropEvent(QDropEvent *event)
{
    if (event->source() != this){
        qWarning("Dropping item not from that widget... aborting!");
        event->ignore();
        return;
    }

    QTreeWidgetItem* dropped_item = currentItem();
    QTreeWidgetItem* target_item = itemAt(event->pos());

    if (!target_item){
        event->ignore();
        return;
    }

    GlobalMessageBox msg;
    int rval = msg.showYesNo(tr("Déplacer?"), tr("Voulez-vous vraiment déplacer") + " \"" + dropped_item->text(0) + "\" " + tr("vers") + " \"" + target_item->text(0) + "\" ?");
    if (rval == QMessageBox::Yes){
        emit moveRequest(dropped_item, target_item);
        event->accept();
    }
    event->ignore();

}

void ProjectNavigatorTree::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() != this){
        qWarning("Dragging item not from that widget... aborting!");
        event->ignore();
        return;
    }

    QTreeWidgetItem* dragged_item = currentItem();
    TeraDataTypes dragged_item_type = m_projNav->getItemType(dragged_item);

    if (dragged_item_type == TERADATA_PROJECT || dragged_item_type == TERADATA_GROUP){
        // Projects and participants groups can't be dragged!
        event->ignore();
        return;
    }

    // Check allowed types
    QTreeWidgetItem* target_item = itemAt(event->pos());
    if (target_item){
        if (dragged_item->parent() == target_item){
            event->ignore();
            return;
        }

        TeraDataTypes target_item_type = m_projNav->getItemType(target_item);
        /*if (dragged_item_type == TERADATA_GROUP){
            if (target_item_type == TERADATA_PROJECT){
                event->accept();
                return;
            }
        }*/

        if (dragged_item_type == TERADATA_PARTICIPANT){
            if (target_item_type == TERADATA_PROJECT || target_item_type == TERADATA_GROUP){
                // Check if target item is in the same project
                QTreeWidgetItem* target_project;
                if (target_item_type == TERADATA_PROJECT){
                    // We are already dragging over a project
                    target_project = target_item;
                }else{
                    target_project = m_projNav->getProjectForItem(target_item);
                }
                QTreeWidgetItem* dragged_project = m_projNav->getProjectForItem(dragged_item);
                if (target_project == dragged_project){
                    event->accept();
                    return;
                }
            }
        }
    }

    event->ignore();

}
