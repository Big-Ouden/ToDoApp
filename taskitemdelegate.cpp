#include "taskitemdelegate.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>

/**
 * @file taskitemdelegate.cpp
 * @brief Délégué pour l'édition in-place des colonnes de Task.
 */

TaskItemDelegate::TaskItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *TaskItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if (index.column() == 0) {
        return new QLineEdit(parent);
    } else if (index.column() == 2) {
        return new QDateEdit(parent);
    } else if (index.column() == 3) {
        QComboBox *cb = new QComboBox(parent);
        cb->addItem("Low");
        cb->addItem("Medium");
        cb->addItem("High");
        cb->addItem("Critical");
        return cb;
    }
    return QStyledItemDelegate::createEditor(parent, QStyleOptionViewItem(), index);
}

void TaskItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant v = index.model()->data(index, Qt::EditRole);
    if (QLineEdit *le = qobject_cast<QLineEdit*>(editor)) {
        le->setText(v.toString());
    } else if (QDateEdit *de = qobject_cast<QDateEdit*>(editor)) {
        de->setDate(QDate::fromString(v.toString(), Qt::ISODate));
    } else if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        int idx = cb->findText(v.toString());
        if (idx >= 0) cb->setCurrentIndex(idx);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void TaskItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QLineEdit *le = qobject_cast<QLineEdit*>(editor)) {
        model->setData(index, le->text(), Qt::EditRole);
    } else if (QDateEdit *de = qobject_cast<QDateEdit*>(editor)) {
        model->setData(index, de->date().toString(Qt::ISODate), Qt::EditRole);
    } else if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, cb->currentText(), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
