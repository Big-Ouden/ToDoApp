#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QDate>
#include <QMap>

class TaskModel;

/**
 * @brief Widget interne affichant le contenu de la heatmap.
 */
class HeatmapContent : public QWidget
{
    Q_OBJECT

public:
    explicit HeatmapContent(TaskModel *model, QWidget *parent = nullptr);
    
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void calculateActivityData();
    QColor getColorForActivity(int count) const;
    QString getTooltipForDate(const QDate &date) const;

    TaskModel *m_model;
    QMap<QDate, int> m_activityData;
    int m_cellSize;
    int m_cellSpacing;
    QDate m_hoveredDate;
};

/**
 * @brief Widget affichant une heatmap de productivité (style GitHub) avec scroll.
 * Affiche l'activité quotidienne sur les 12 derniers mois.
 */
class HeatmapWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit HeatmapWidget(TaskModel *model, QWidget *parent = nullptr);

private:
    HeatmapContent *m_content;
};

#endif // HEATMAPWIDGET_H
