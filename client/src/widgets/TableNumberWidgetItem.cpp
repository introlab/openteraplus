#include "TableNumberWidgetItem.h"

bool TableNumberWidgetItem::operator<(const QTableWidgetItem &other) const
{
    QString current_value = this->text();
    QString other_value = other.text();

    // Convert to number and compate
    return current_value.toFloat() < other_value.toFloat();
}

