#include "taskitemdelegate.h"
#include "taskmodel.h"
#include "task.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QHelpEvent>
#include <QToolTip>
#include <QAbstractItemView>
#include <QApplication>
#include <QPalette>

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

void TaskItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Agrandir la géométrie de l'éditeur pour qu'il soit plus visible
    QRect rect = option.rect;
    
    if (index.column() == 0) {
        // Pour le titre, utiliser plus de largeur
        rect.setWidth(qMax(rect.width(), 300));
    } else if (index.column() == 1) {
        // Pour la date, taille fixe appropriée
        rect.setWidth(qMax(rect.width(), 150));
    } else if (index.column() == 2 || index.column() == 3) {
        // Pour priorité et statut, taille minimale
        rect.setWidth(qMax(rect.width(), 120));
    }
    
    // Augmenter légèrement la hauteur pour une meilleure lisibilité
    rect.setHeight(qMax(rect.height(), 25));
    
    editor->setGeometry(rect);
}

bool TaskItemDelegate::helpEvent(QHelpEvent *event,
                                  QAbstractItemView *view,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    if (!event || !view)
        return false;

    if (event->type() == QEvent::ToolTip) {
        if (index.isValid()) {
            QString tooltip = generateRichTooltip(index);
            if (!tooltip.isEmpty()) {
                QToolTip::showText(event->globalPos(), tooltip, view);
                return true;
            }
        }
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QString TaskItemDelegate::generateRichTooltip(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    // Récupérer la tâche depuis le modèle
    const TaskModel *taskModel = qobject_cast<const TaskModel*>(index.model());
    if (!taskModel)
        return QString();

    Task *task = taskModel->getTask(index);
    if (!task)
        return QString();

    // Détecter le thème sombre
    QPalette palette = qApp->palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    QString textColor = isDarkTheme ? "#e0e0e0" : "#000000";
    QString mutedColor = isDarkTheme ? "#a0a0a0" : "#555";
    
    // Construire un tooltip riche en HTML
    QString html = "<html><body style='padding: 5px;'>";
    
    // Titre
    html += "<p><b style='font-size: 14px; color: " + textColor + ";'>" + task->title().toHtmlEscaped() + "</b></p>";
    
    // Description (tronquée si trop longue)
    if (!task->description().isEmpty()) {
        QString desc = task->description();
        if (desc.length() > 200) {
            desc = desc.left(200) + "...";
        }
        html += "<p style='color: " + mutedColor + ";'>" + desc.toHtmlEscaped() + "</p>";
    }
    
    QString hrColor = isDarkTheme ? "#444" : "#ddd";
    html += "<hr style='border: 1px solid " + hrColor + ";'/>";
    
    // Informations
    html += "<table cellpadding='2' cellspacing='0'>";
    
    // Priorité avec couleur
    QString priorityColor;
    QString priorityText;
    switch (task->priority()) {
        case Priority::LOW:
            priorityColor = "#4CAF50";
            priorityText = QObject::tr("Basse");
            break;
        case Priority::MEDIUM:
            priorityColor = "#FFC107";
            priorityText = QObject::tr("Moyenne");
            break;
        case Priority::HIGH:
            priorityColor = "#FF9800";
            priorityText = QObject::tr("Haute");
            break;
        case Priority::CRITICAL:
            priorityColor = "#F44336";
            priorityText = QObject::tr("Critique");
            break;
    }
    html += "<tr><td><b>" + QObject::tr("Priorité:") + "</b></td><td><span style='color: " + priorityColor + ";'>● " + priorityText + "</span></td></tr>";
    
    // Statut
    QString statusText;
    switch (task->status()) {
        case Status::NOTSTARTED: statusText = QObject::tr("Non commencé"); break;
        case Status::INPROGRESS: statusText = QObject::tr("En cours"); break;
        case Status::COMPLETED: statusText = QObject::tr("Terminé"); break;
        case Status::CANCELLED: statusText = QObject::tr("Annulé"); break;
    }
    html += "<tr><td><b>" + QObject::tr("Statut:") + "</b></td><td>" + statusText + "</td></tr>";
    
    // Date d'échéance
    if (task->dueDate().isValid()) {
        QString dateStr = task->dueDate().toString("dd/MM/yyyy");
        bool isOverdue = task->isOverdue();
        if (isOverdue) {
            html += "<tr><td><b>" + QObject::tr("Échéance:") + "</b></td><td><span style='color: red;'>⚠ " + dateStr + " (" + QObject::tr("en retard") + ")</span></td></tr>";
        } else {
            html += "<tr><td><b>" + QObject::tr("Échéance:") + "</b></td><td>" + dateStr + "</td></tr>";
        }
    }
    
    // Temps estimé vs réel
    if (task->estimatedMinutes() > 0 || task->actualMinutes() > 0) {
        QString timeStr;
        if (task->estimatedMinutes() > 0) {
            int h = task->estimatedMinutes() / 60;
            int m = task->estimatedMinutes() % 60;
            timeStr = QObject::tr("Estimé: %1h%2").arg(h).arg(m, 2, 10, QChar('0'));
        }
        if (task->actualMinutes() > 0) {
            int h = task->actualMinutes() / 60;
            int m = task->actualMinutes() % 60;
            if (!timeStr.isEmpty()) timeStr += " | ";
            timeStr += QObject::tr("Réel: %1h%2").arg(h).arg(m, 2, 10, QChar('0'));
        }
        if (!timeStr.isEmpty()) {
            html += "<tr><td><b>" + QObject::tr("Temps:") + "</b></td><td>" + timeStr + "</td></tr>";
        }
    }
    
    // Tags
    if (!task->tags().isEmpty()) {
        QString tagsStr = task->tags().join(", ");
        html += "<tr><td><b>" + QObject::tr("Tags:") + "</b></td><td><i>" + tagsStr.toHtmlEscaped() + "</i></td></tr>";
    }
    
    // Sous-tâches
    if (!task->subtasks().isEmpty()) {
        int total = task->subtasks().count();
        int completed = 0;
        for (Task *sub : task->subtasks()) {
            if (sub->isCompleted()) completed++;
        }
        html += "<tr><td><b>" + QObject::tr("Sous-tâches:") + "</b></td><td>" + QObject::tr("%1/%2 terminées").arg(completed).arg(total) + "</td></tr>";
    }
    
    html += "</table>";
    html += "</body></html>";
    
    return html;
}
