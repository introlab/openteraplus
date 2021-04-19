#ifndef TABLENUMBERWIDGETITEM_H
#define TABLENUMBERWIDGETITEM_H

#include <QTableWidgetItem>
#include <QObject>

class TableNumberWidgetItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem; // Use base class constructors

    bool operator<(const QTableWidgetItem &other) const override;
};

#endif // TABLENUMBERWIDGETITEM_H
