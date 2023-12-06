#ifndef TABLEDATEWIDGETITEM_H
#define TABLEDATEWIDGETITEM_H

#include <QTableWidgetItem>
#include <QDateTime>
#include <QObject>
#include <QDateTime>

class TableDateWidgetItem : public QTableWidgetItem
{
    public:

        using QTableWidgetItem::QTableWidgetItem; // Use base class constructors
        explicit TableDateWidgetItem(const QDateTime& date);

        bool operator<(const QTableWidgetItem &other) const override;

        void setDate(const QVariant &date_var);

    private:
        QDateTime m_date;
};


#endif // TABLEDATEWIDGETITEM_H
