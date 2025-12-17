#ifndef KANBANVIEW_H
#define KANBANVIEW_H

#include <QWidget>
#include <QMap>
#include <QListWidget>
#include "status.h"

class TaskModel;
class QVBoxLayout;
class QListWidgetItem;
class Task;

/**
 * @brief QListWidget personnalisé qui détecte les drops pour le Kanban.
 */
class KanbanColumn : public QListWidget
{
    Q_OBJECT

public:
    explicit KanbanColumn(Status status, QWidget *parent = nullptr)
        : QListWidget(parent), m_status(status) {}
    
    Status columnStatus() const { return m_status; }

signals:
    void itemDroppedInColumn(Status targetStatus);

protected:
    void dropEvent(QDropEvent *event) override
    {
        QListWidget::dropEvent(event);
        emit itemDroppedInColumn(m_status);
    }

private:
    Status m_status;
};

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
    void onColumnDropped(Status targetStatus);

private:
    void setupUI();
    void refreshColumns();
    void populateColumn(Status status);
    QString formatTaskCard(Task *task) const;

    TaskModel *m_model;
    QMap<Status, KanbanColumn*> m_columns;
    QMap<QListWidgetItem*, Task*> m_itemTaskMap;
};

#endif // KANBANVIEW_H
