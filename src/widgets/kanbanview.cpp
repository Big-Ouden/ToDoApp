#include "kanbanview.h"
#include "taskmodel.h"
#include "task.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QFrame>
#include <QApplication>
#include <QPalette>

KanbanView::KanbanView(TaskModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    setupUI();
    
    connect(m_model, &QAbstractItemModel::dataChanged, this, &KanbanView::onModelDataChanged);
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &KanbanView::onModelDataChanged);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &KanbanView::onModelDataChanged);
    connect(m_model, &QAbstractItemModel::modelReset, this, &KanbanView::onModelDataChanged);
    
    refreshColumns();
}

void KanbanView::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Créer une colonne pour chaque statut
    QList<QPair<Status, QString>> statuses = {
        {Status::NOTSTARTED, tr("À faire")},
        {Status::INPROGRESS, tr("En cours")},
        {Status::COMPLETED, tr("Terminé")},
        {Status::CANCELLED, tr("Annulé")}
    };
    
    for (const auto &statusPair : statuses) {
        Status status = statusPair.first;
        QString title = statusPair.second;
        
        // Conteneur de la colonne
        QWidget *columnWidget = new QWidget();
        QVBoxLayout *columnLayout = new QVBoxLayout(columnWidget);
        columnLayout->setContentsMargins(0, 0, 0, 0);
        columnLayout->setSpacing(5);
        
        // En-tête de la colonne
        QLabel *header = new QLabel(title);
        header->setObjectName("kanbanColumnHeader");
        header->setAlignment(Qt::AlignCenter);
        columnLayout->addWidget(header);
        
        // Liste des tâches - utiliser notre QListWidget personnalisé
        KanbanColumn *listWidget = new KanbanColumn(status);
        listWidget->setObjectName(QString("kanbanColumn_%1").arg(static_cast<int>(status)));
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setDefaultDropAction(Qt::MoveAction);
        listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        listWidget->setSpacing(5);
        listWidget->setMinimumWidth(280);
        listWidget->setMaximumWidth(400);
        listWidget->setAcceptDrops(true);
        
        connect(listWidget, &QListWidget::itemDoubleClicked,
                this, &KanbanView::onItemDoubleClicked);
        
        // Connecter le signal de drop personnalisé
        connect(listWidget, &KanbanColumn::itemDroppedInColumn,
                this, &KanbanView::onColumnDropped);
        
        m_columns[status] = listWidget;
        columnLayout->addWidget(listWidget);
        
        mainLayout->addWidget(columnWidget, 1);
    }
}

void KanbanView::refreshColumns()
{
    // Vider toutes les colonnes
    for (auto it = m_columns.begin(); it != m_columns.end(); ++it) {
        it.value()->clear();
    }
    m_itemTaskMap.clear();
    
    // Remplir chaque colonne
    for (auto it = m_columns.begin(); it != m_columns.end(); ++it) {
        populateColumn(it.key());
    }
}

void KanbanView::populateColumn(Status status)
{
    KanbanColumn *column = m_columns[status];
    if (!column) return;
    
    // Parcourir toutes les tâches du modèle
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex idx = m_model->index(row, 0, QModelIndex());
        Task *task = m_model->getTask(idx);
        
        if (task && task->status() == status) {
            QListWidgetItem *item = new QListWidgetItem();
            item->setText(formatTaskCard(task));
            
            // Stocker un pointeur vers la tâche dans l'item pour le retrouver après drag&drop
            item->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(task)));
            
            // Les couleurs de priorité seront gérées par le QSS externe
            
            column->addItem(item);
            m_itemTaskMap[item] = task;
        }
    }
}

QString KanbanView::formatTaskCard(Task *task) const
{
    QString card = task->title();
    
    if (task->dueDate().isValid()) {
        card += "\n📅 " + task->dueDate().toString("dd/MM/yyyy");
    }
    
    if (task->estimatedMinutes() > 0) {
        int hours = task->estimatedMinutes() / 60;
        int mins = task->estimatedMinutes() % 60;
        if (hours > 0) {
            card += QString("\n⏱ %1h%2").arg(hours).arg(mins, 2, 10, QChar('0'));
        } else {
            card += QString("\n⏱ %1 min").arg(mins);
        }
    }
    
    if (!task->tags().isEmpty()) {
        card += "\n🏷 " + task->tags().join(", ");
    }
    
    return card;
}

void KanbanView::onModelDataChanged()
{
    refreshColumns();
}

void KanbanView::onItemDoubleClicked(QListWidgetItem *item)
{
    Task *task = m_itemTaskMap.value(item, nullptr);
    if (task) {
        // Émettre un signal pour ouvrir les détails
        // TODO: connecter au détail widget
    }
}

void KanbanView::onColumnDropped(Status targetStatus)
{
    // Trouver dans quelle colonne l'item a été déposé
    KanbanColumn *targetColumn = m_columns[targetStatus];
    if (!targetColumn) return;
    
    // Parcourir tous les items de la colonne cible
    for (int i = 0; i < targetColumn->count(); ++i) {
        QListWidgetItem *item = targetColumn->item(i);
        
        // Récupérer le pointeur de la tâche stocké dans l'item
        QVariant taskData = item->data(Qt::UserRole);
        if (taskData.isValid()) {
            Task *task = reinterpret_cast<Task*>(taskData.value<quintptr>());
            
            // Si la tâche a un statut différent, on doit la mettre à jour
            if (task && task->status() != targetStatus) {
                // Mettre à jour le statut
                task->setStatus(targetStatus);
                
                // Notifier le modèle
                QModelIndex idx = m_model->getIndexForTask(task);
                if (idx.isValid()) {
                    emit m_model->dataChanged(idx, idx);
                }
                
                // Rafraîchir toutes les colonnes
                refreshColumns();
                return;
            }
        }
    }
}
