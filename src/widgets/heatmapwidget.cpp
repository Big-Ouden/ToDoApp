#include "heatmapwidget.h"
#include "taskmodel.h"
#include "task.h"
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QDate>
#include <QVBoxLayout>

/**
 * @brief Constructeur du contenu de la heatmap
 * @param model Modèle de tâches à visualiser
 * @param parent Widget parent
 * @note Connecte automatiquement aux changements du modèle
 */
HeatmapContent::HeatmapContent(TaskModel *model, QWidget *parent)
    : QWidget(parent),
      m_model(model),
      m_cellSize(12),
      m_cellSpacing(3)
{
    setMouseTracking(true);
    calculateActivityData();
    
    connect(m_model, &QAbstractItemModel::dataChanged, this, [this]() {
        calculateActivityData();
        update();
    });
    connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
        calculateActivityData();
        update();
    });
}

/**
 * @brief Retourne la taille recommandée pour afficher 12 mois
 * @return Taille en pixels (53 semaines x 7 jours)
 */
QSize HeatmapContent::sizeHint() const
{
    int weeks = 53;  // ~12 mois
    int days = 7;
    int width = weeks * (m_cellSize + m_cellSpacing) + 100;  // Plus de marge
    int height = days * (m_cellSize + m_cellSpacing) + 100;
    return QSize(width, height);
}

QSize HeatmapContent::minimumSizeHint() const
{
    return sizeHint();  // Force la taille complète
}

/**
 * @brief Dessine la heatmap d'activité
 * @param event Événement de peinture
 * @note Affiche les 12 derniers mois sous forme de grille colorée
 */
void HeatmapContent::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Titre
    painter.setPen(palette().text().color());
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(10, 20, tr("Activité des 12 derniers mois"));
    
    // Calculer la date de début (il y a ~12 mois)
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-364);  // ~52 semaines
    
    // Ajuster au dimanche précédent
    int daysToSunday = startDate.dayOfWeek() % 7;
    startDate = startDate.addDays(-daysToSunday);
    
    int offsetX = 40;
    int offsetY = 40;
    
    // Dessiner les labels des jours
    QStringList dayLabels = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
    painter.setFont(QFont("Arial", 8));
    for (int day = 0; day < 7; ++day) {
        int y = offsetY + day * (m_cellSize + m_cellSpacing);
        painter.drawText(5, y + m_cellSize, dayLabels[day]);
    }
    
    // Dessiner les cellules
    QDate currentDate = startDate;
    int week = 0;
    
    while (currentDate <= endDate) {
        int dayOfWeek = currentDate.dayOfWeek() % 7;
        int x = offsetX + week * (m_cellSize + m_cellSpacing);
        int y = offsetY + dayOfWeek * (m_cellSize + m_cellSpacing);
        
        int activity = m_activityData.value(currentDate, 0);
        QColor color = getColorForActivity(activity);
        
        painter.fillRect(x, y, m_cellSize, m_cellSize, color);
        
        // Bordure
        if (currentDate == m_hoveredDate) {
            painter.setPen(QPen(palette().highlight().color(), 2));
        } else {
            painter.setPen(QPen(palette().mid().color(), 1));
        }
        painter.drawRect(x, y, m_cellSize, m_cellSize);
        
        // Passer au jour suivant
        currentDate = currentDate.addDays(1);
        if (dayOfWeek == 6) {  // Samedi -> nouvelle semaine
            week++;
        }
    }
    
    // Légende
    int legendY = offsetY + 7 * (m_cellSize + m_cellSpacing) + 20;
    painter.setPen(palette().text().color());
    painter.setFont(QFont("Arial", 9));
    painter.drawText(offsetX, legendY, tr("Moins"));
    
    for (int i = 0; i <= 4; ++i) {
        int x = offsetX + 50 + i * (m_cellSize + m_cellSpacing);
        QColor color = getColorForActivity(i * 3);
        painter.fillRect(x, legendY - 10, m_cellSize, m_cellSize, color);
        painter.setPen(QPen(palette().mid().color(), 1));
        painter.drawRect(x, legendY - 10, m_cellSize, m_cellSize);
    }
    
    painter.setPen(palette().text().color());
    painter.drawText(offsetX + 50 + 5 * (m_cellSize + m_cellSpacing), legendY, tr("Plus"));
}

void HeatmapContent::mouseMoveEvent(QMouseEvent *event)
{
    int offsetX = 40;
    int offsetY = 40;
    
    int x = event->pos().x() - offsetX;
    int y = event->pos().y() - offsetY;
    
    if (x < 0 || y < 0) {
        m_hoveredDate = QDate();
        update();
        return;
    }
    
    int week = x / (m_cellSize + m_cellSpacing);
    int day = y / (m_cellSize + m_cellSpacing);
    
    if (day >= 0 && day < 7) {
        QDate endDate = QDate::currentDate();
        QDate startDate = endDate.addDays(-364);
        int daysToSunday = startDate.dayOfWeek() % 7;
        startDate = startDate.addDays(-daysToSunday);
        
        QDate hoveredDate = startDate.addDays(week * 7 + day);
        
        if (hoveredDate <= endDate) {
            m_hoveredDate = hoveredDate;
            
            // Afficher le tooltip
            QString tooltip = getTooltipForDate(hoveredDate);
            QToolTip::showText(event->globalPosition().toPoint(), tooltip, this);
            
            update();
            return;
        }
    }
    
    m_hoveredDate = QDate();
    update();
}

void HeatmapContent::calculateActivityData()
{
    m_activityData.clear();
    
    // Parcourir toutes les tâches
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex idx = m_model->index(row, 0, QModelIndex());
        Task *task = m_model->getTask(idx);
        
        if (task && task->isCompleted()) {
            // Utiliser la date de complétion si disponible, sinon la date d'échéance
            QDate completionDate = task->completionDate();
            if (!completionDate.isValid()) {
                completionDate = task->dueDate();
            }
            if (completionDate.isValid()) {
                m_activityData[completionDate]++;
            }
        }
    }
}

QColor HeatmapContent::getColorForActivity(int count) const
{
    // Utiliser des couleurs adaptées au thème (avec alpha pour transparence)
    if (count == 0) {
        // Cellule vide: utiliser la couleur de base de l'interface
        return palette().button().color();
    } else if (count <= 2) {
        return QColor(76, 175, 80, 100);  // Vert clair avec alpha
    } else if (count <= 5) {
        return QColor(76, 175, 80, 150);  // Vert moyen
    } else if (count <= 9) {
        return QColor(76, 175, 80, 200);  // Vert foncé
    } else {
        return QColor(76, 175, 80, 255);  // Vert très foncé
    }
}

QString HeatmapContent::getTooltipForDate(const QDate &date) const
{
    int count = m_activityData.value(date, 0);
    QString dateStr = date.toString("dddd d MMMM yyyy");
    
    if (count == 0) {
        return tr("%1\nAucune tâche complétée").arg(dateStr);
    } else if (count == 1) {
        return tr("%1\n1 tâche complétée").arg(dateStr);
    } else {
        return tr("%1\n%2 tâches complétées").arg(dateStr).arg(count);
    }
}

// Wrapper class implementation
HeatmapWidget::HeatmapWidget(TaskModel *model, QWidget *parent)
    : QScrollArea(parent)
{
    m_content = new HeatmapContent(model, this);
    
    setWidget(m_content);
    setWidgetResizable(false);  // Keep exact size
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameShape(QFrame::NoFrame);
    
    // Set minimum size for the scroll area
    setMinimumSize(400, 200);
}
