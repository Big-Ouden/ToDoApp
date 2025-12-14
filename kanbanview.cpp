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
        header->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; padding: 8px; background-color: palette(window); border-radius: 4px; }");
        header->setAlignment(Qt::AlignCenter);
        columnLayout->addWidget(header);
        
        // Liste des tâches
        QListWidget *listWidget = new QListWidget();
        listWidget->setObjectName(QString("kanbanColumn_%1").arg(static_cast<int>(status)));
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setDefaultDropAction(Qt::MoveAction);
        listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        listWidget->setSpacing(5);
        listWidget->setMinimumWidth(280);
        listWidget->setMaximumWidth(400);
        
        // Style adapté au thème (utilise palette pour s'adapter au thème sombre)
        listWidget->setStyleSheet(
            "QListWidget { "
            "  background-color: palette(base); "
            "  border: 1px solid palette(mid); "
            "  border-radius: 4px; "
            "  color: palette(text); "
            "} "
            "QListWidget::item { "
            "  background-color: palette(window); "
            "  color: palette(text); "
            "  margin: 3px; "
            "  padding: 8px; "
            "  border-radius: 4px; "
            "  border: 1px solid palette(mid); "
            "} "
            "QListWidget::item:hover { "
            "  background-color: palette(midlight); "
            "  color: palette(text); "
            "} "
            "QListWidget::item:selected { "
            "  background-color: palette(highlight); "
            "  color: palette(highlighted-text); "
            "}"
        );
        
        connect(listWidget, &QListWidget::itemDoubleClicked,
                this, &KanbanView::onItemDoubleClicked);
        
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
    QListWidget *column = m_columns[status];
    if (!column) return;
    
    // Parcourir toutes les tâches du modèle
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex idx = m_model->index(row, 0, QModelIndex());
        Task *task = m_model->getTask(idx);
        
        if (task && task->status() == status) {
            QListWidgetItem *item = new QListWidgetItem();
            item->setText(formatTaskCard(task));
            
            // Couleur selon la priorité (adaptée au thème)
            QPalette palette = qApp->palette();
            bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
            
            QColor bgColor;
            if (isDarkTheme) {
                switch (task->priority()) {
                    case Priority::CRITICAL: bgColor = QColor(139, 50, 50, 120); break;
                    case Priority::HIGH:     bgColor = QColor(139, 90, 43, 120); break;
                    case Priority::MEDIUM:   bgColor = QColor(139, 139, 43, 120); break;
                    case Priority::LOW:      bgColor = QColor(50, 100, 50, 120); break;
                }
            } else {
                switch (task->priority()) {
                    case Priority::CRITICAL: bgColor = QColor(255, 100, 100, 80); break;
                    case Priority::HIGH:     bgColor = QColor(255, 152, 0, 80); break;
                    case Priority::MEDIUM:   bgColor = QColor(255, 235, 59, 80); break;
                    case Priority::LOW:      bgColor = QColor(76, 175, 80, 80); break;
                }
            }
            item->setBackground(bgColor);
            
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

void KanbanView::onItemDropped(QListWidgetItem *item)
{
    // Gérer le drag & drop entre colonnes
    // Le changement de statut sera détecté par la colonne de destination
}
