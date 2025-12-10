#ifndef TASKITEMDELEGATE_H
#define TASKITEMDELEGATE_H

#pragma once

#include <QStyledItemDelegate>
#include <QWidget>

class TaskItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskItemDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
};

#endif // TASKITEMDELEGATE_H
