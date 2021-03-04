#ifndef GENERALSORTERMODEL_H
#define GENERALSORTERMODEL_H

#include <QSortFilterProxyModel>
#include <QDate>

#include <QObject>

class GeneralSorterModel : public QSortFilterProxyModel
{
public:
    GeneralSorterModel(QObject *parent);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif // GENERALSORTERMODEL_H
