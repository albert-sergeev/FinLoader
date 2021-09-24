#ifndef DATEITEMDELEGATE_H
#define DATEITEMDELEGATE_H

#include <QStyledItemDelegate>

class DateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
//public:
//    DateItemDelegate();

public:
    using QStyledItemDelegate::QStyledItemDelegate;

//    void paint(QPainter *painter, const QStyleOptionViewItem &option,
//               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private slots:
    void commitAndCloseEditor();
    void commitAndCloseTimeEditor();
};

#endif // DATEITEMDELEGATE_H
