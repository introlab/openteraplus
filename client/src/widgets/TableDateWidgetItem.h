#ifndef TABLEDATEWIDGETITEM_H
#define TABLEDATEWIDGETITEM_H

#include <QTableWidgetItem>
#include <QDateTime>
#include <QObject>

class TableDateWidgetItem : public QTableWidgetItem
{
public:

  using QTableWidgetItem::QTableWidgetItem; // Use base class constructors

  bool operator<(const QTableWidgetItem &other) const override;
};

#endif // TABLEDATEWIDGETITEM_H
