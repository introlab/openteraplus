#include "GeneralSorterModel.h"

GeneralSorterModel::GeneralSorterModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{

}

bool GeneralSorterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftVariant = sourceModel()->data(source_left);
    QVariant rightVariant = sourceModel()->data(source_right);

    // Null elements management - always before everything
    if (leftVariant.isNull() || rightVariant.isNull()){
        if ((!leftVariant.isNull() && rightVariant.isNull()) ||
            (leftVariant.isNull() && rightVariant.isNull())){
            return false;
        }
        return true;
    }

    QString leftData = sourceModel()->data(source_left).toString();
    QString rightData = sourceModel()->data(source_right).toString();

    // Numbers sorting
    QRegExp numValidator("\\d*");
    if (numValidator.exactMatch(leftData) && numValidator.exactMatch(rightData)){
        if (leftData.contains(".") || leftData.contains(",") || rightData.contains(".") || rightData.contains(",")){
            // Floating-point number
            leftData = leftData.replace(",",".");
            rightData = rightData.replace(",",".");
            return leftData.toFloat() < rightData.toFloat();
        }else{
            // Integer
            return leftData.toInt() < rightData.toInt();
        }
    }

    if (leftData.endsWith(" %") && rightData.endsWith(" %")){
        // Percentages
        return leftData.leftRef(leftData.length()-2).toFloat() < rightData.leftRef(rightData.length()-2).toFloat();
    }

    // Date sorting?
    QRegExp dateValidator("\\d\\d\\d\\d-\\d\\d-\\d*");
    if (dateValidator.exactMatch(leftData) && dateValidator.exactMatch(rightData)){
        // Oui, on doit donc convertir...
        QDate leftDate = QDate::fromString(leftData, "yyyy-MM-dd");
        QDate rightDate = QDate::fromString(rightData, "yyyy-MM-dd");

        return leftDate < rightDate;
    }

    dateValidator.setPattern("\\d\\d\\/\\d\\d\\/\\d\\d \\d*:\\d*:\\d*");
    if (dateValidator.exactMatch(leftData) && dateValidator.exactMatch(rightData)){
        // Oui, on doit donc convertir...
        QDateTime leftDate = QDateTime::fromString(leftData, "yy/MM/dd hh:mm:ss");
        QDateTime rightDate = QDateTime::fromString(rightData, "yy/MM/dd hh:mm:ss");

        return leftDate < rightDate;
    }

    // Texte only - default sorter
    return QSortFilterProxyModel::lessThan(source_left,source_right);
}

