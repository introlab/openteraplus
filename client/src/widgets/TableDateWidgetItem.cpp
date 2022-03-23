#include "TableDateWidgetItem.h"


bool TableDateWidgetItem::operator<(const QTableWidgetItem &other) const
{
    QString current_value = this->text();
    QString other_value = other.text();

    // Convert to date and compate
    return (QDateTime::fromString(current_value, "dd-MM-yyyy hh:mm:ss") < QDateTime::fromString(other_value, "dd-MM-yyyy hh:mm:ss"));
}

void TableDateWidgetItem::setDate(const QVariant &date_var)
{
    QDateTime date_value = date_var.toDateTime().toLocalTime();
    setText(date_value.toString("dd-MM-yyyy hh:mm:ss"));
}

