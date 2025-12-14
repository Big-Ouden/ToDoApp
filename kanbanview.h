#ifndef KANBANVIEW_H
#define KANBANVIEW_H

#include <QWidget>
#include <QMap>
#include "status.h"

class TaskModel;
class QVBoxLayout;
class QListWidget;
class QListWidgetItem;
class Task;

/**
 * @brief Vue Kanban avec colonnes par statut et drag & drop.
 */
class KanbanView : public QWidget
{
    Q_OBJECT

public:
    explicit KanbanView(TaskModel *model, QWidget *parent = nullptr);

private slots:
    void onModelDataChanged();
    void onItemDoubleClicked(QListWidgetItem *item);
    void onItemDropped(QListWidgetItem *item);

private:
    void setupUI();
    void refreshColumns();
    void populateColumn(Status status);
    QString formatTaskCard(Task *task) const;

    TaskModel *m_model;
    QMap<Status, QListWidget*> m_columns;
    QMap<QListWidgetItem*, Task*> m_itemTaskMap;
};

#endif // KANBANVIEW_H
