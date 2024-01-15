#include "TableDateWidgetItem.h"

TableDateWidgetItem::TableDateWidgetItem(const QDateTime &date)
{
    setDate(date);
}

bool TableDateWidgetItem::operator<(const QTableWidgetItem &other) const
{
    const TableDateWidgetItem* other_item = dynamic_cast<const TableDateWidgetItem*>(&other);
    QDateTime other_date;
    if (!other_item->m_date.isValid()){
        other_date = QDateTime::fromString(other.text(), "dd-MM-yyyy hh:mm:ss");
    }else{
        other_date = other_item->m_date;
    }

    /*QString current_value = this->text();
    QString other_value = other.text();

    // Convert to date and compate
    return (QDateTime::fromString(current_value, "dd-MM-yyyy hh:mm:ss") < QDateTime::fromString(other_value, "dd-MM-yyyy hh:mm:ss"));*/


    if (m_date.isValid())
        return (m_date < other_date);
    else
        return (QDateTime::fromString(text(), "dd-MM-yyyy hh:mm:ss") < other_date);
}

void TableDateWidgetItem::setDate(const QVariant &date_var)
{
    m_date = date_var.toDateTime().toLocalTime();
    setText(m_date.toString("dd-MM-yyyy hh:mm:ss"));
}

