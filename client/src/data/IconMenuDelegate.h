#ifndef ICONMENUDELEGATE_H
#define ICONMENUDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

class IconMenuDelegate : public QStyledItemDelegate
{
public:
    explicit IconMenuDelegate(int gridHeight, QObject *parent);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    int m_gridHeight;
};

#endif // ICONMENUDELEGATE_H
