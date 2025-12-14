#ifndef TASKITEMDELEGATE_H
#define TASKITEMDELEGATE_H

#pragma once

#include <QStyledItemDelegate>
#include <QWidget>

/**
 * @brief Délégué pour l'édition des cellules dans la vue.
 */
class TaskItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /** @brief Constructeur du délégué */
    explicit TaskItemDelegate(QObject *parent = nullptr);

    /**
     * @brief Crée un widget d'édition pour une cellule.
     * @param parent Widget parent
     * @param option Options de style
     * @param index Index de la cellule
     * @return Widget d'édition créé
     */
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /**
     * @brief Charge les données du modèle dans l'éditeur.
     * @param editor Widget d'édition
     * @param index Index de la cellule
     */
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;

    /**
     * @brief Enregistre les données de l'éditeur dans le modèle.
     * @param editor Widget d'édition
     * @param model Modèle de données
     * @param index Index de la cellule
     */
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    /**
     * @brief Gère l'affichage d'aide (tooltip enrichi).
     * @param event Événement
     * @param view Vue
     * @param option Options de style
     * @param index Index de la cellule
     * @return true si l'événement est géré
     */
    bool helpEvent(QHelpEvent *event,
                   QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;

private:
    QString generateRichTooltip(const QModelIndex &index) const;
};

#endif // TASKITEMDELEGATE_H
