#include "IconMenuDelegate.h"

IconMenuDelegate::IconMenuDelegate(int gridHeight, QObject *parent)
    : QStyledItemDelegate{parent}
{
    m_gridHeight = gridHeight;
}

QSize IconMenuDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fm(option.font);
    const QAbstractItemModel* model = index.model();
    QString Text = model->data(index, Qt::DisplayRole).toString();
    QRect neededsize = fm.boundingRect( option.rect, Qt::TextWordWrap,Text );
    return QSize(option.rect.width(), neededsize.height() + m_gridHeight);
}
