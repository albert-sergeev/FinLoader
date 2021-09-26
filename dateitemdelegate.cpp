#include "dateitemdelegate.h"
#include <QDate>
#include <QPainter>
#include <QDateEdit>

#include "threadfreecout.h"


//DateItemDelegate::DateItemDelegate()
//{

//}

//-----------------------------------------------------------------------------------------------------
//void DateItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
//           const QModelIndex &index) const
//{
////    if (index.data().canConvert<QDate>()) {

////        QDate dateD = qvariant_cast<QDate>(index.data());

////        painter->save();

////        if (option.state & QStyle::State_Selected)
////            painter->fillRect(option.rect, option.palette.highlight());
////        //////////////////////////////////////
////        QString str = dateD.toString("dd/MM/yyyy");

////        painter->setRenderHint(QPainter::Antialiasing, true);
////        painter->setPen(Qt::PenStyle::SolidLine);
////        painter->setBrush(option.palette.windowText());

////        painter->translate(option.rect.x(), option.rect.y() + option.rect.height()/2);
////        painter->drawText(4,4,str);

////        painter->restore();

////    }
////    else
//    {
//        QStyledItemDelegate::paint(painter, option, index);
//    }
//}
//-----------------------------------------------------------------------------------------------------
QSize DateItemDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    if (index.data().canConvert<QDate>()) {
        QDate dateD = qvariant_cast<QDate>(index.data());
        QDateEdit editor(dateD);
        return editor.sizeHint();
    }
    else if (index.data().canConvert<QTime>()) {
        QTime dateD = qvariant_cast<QTime>(index.data());
        QTimeEdit editor(dateD);
        return editor.sizeHint();
    }
    return QStyledItemDelegate::sizeHint(option, index);
}
//-----------------------------------------------------------------------------------------------------
QWidget * DateItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                      const QModelIndex &index) const
{
    if (index.data().canConvert<QDate>()) {
        QDateEdit * editor = new QDateEdit(parent);
        editor->setCalendarPopup(false);
        connect(editor, &QDateEdit::editingFinished,
                this, &DateItemDelegate::commitAndCloseEditor);
        return editor;
    }
    else if (index.data().canConvert<QTime>()) {
        QTimeEdit * editor = new QTimeEdit(parent);
        //editor->setDisplayFormat("HH:mm:ss");
        editor->setCalendarPopup(false);
        connect(editor, &QTimeEdit::editingFinished,
                this, &DateItemDelegate::commitAndCloseTimeEditor);
        return editor;
    }
    else{
        return nullptr;
        //return QStyledItemDelegate::createEditor(parent, option, index);
    }
}
//-----------------------------------------------------------------------------------------------------
void DateItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.data().canConvert<QDate>()) {
        QDate dateD = qvariant_cast<QDate>(index.data());
        QDateEdit * ed = qobject_cast<QDateEdit *>(editor);
        ed->setDate(dateD);
    }
    else if (index.data().canConvert<QTime>()) {
        QTime dateD = qvariant_cast<QTime>(index.data());
        QTimeEdit * ed = qobject_cast<QTimeEdit *>(editor);
        ed->setTime(dateD);
    }
    else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//-----------------------------------------------------------------------------------------------------
void DateItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    if (index.data().canConvert<QDate>()) {
        QDateEdit * ed = qobject_cast<QDateEdit *>(editor);
        if (ed->date().isValid()){
            model->setData(index, QVariant::fromValue(ed->date()));
        }
    }
    else if (index.data().canConvert<QTime>()) {
        QTimeEdit * ed = qobject_cast<QTimeEdit *>(editor);
        if (ed->time().isValid()){
            model->setData(index, QVariant::fromValue(ed->time()));
        }
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }

}
//-----------------------------------------------------------------------------------------------------
void DateItemDelegate::commitAndCloseEditor()
{
     QDateEdit *editor = qobject_cast<QDateEdit *>(sender());
     emit commitData(editor);
     emit closeEditor(editor);
}
//-----------------------------------------------------------------------------------------------------
void DateItemDelegate::commitAndCloseTimeEditor()
{
     QTimeEdit *editor = qobject_cast<QTimeEdit *>(sender());
     emit commitData(editor);
     emit closeEditor(editor);
}

//-----------------------------------------------------------------------------------------------------

