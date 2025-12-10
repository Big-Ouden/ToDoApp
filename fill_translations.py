#!/usr/bin/env python3
"""Script pour remplir automatiquement les traductions françaises et anglaises"""

import xml.etree.ElementTree as ET
import sys

# Dictionnaire de traductions françaises
translations_fr = {
    # UI de base
    "ToDoApp - Gestionnaire de tâches": "ToDoApp - Gestionnaire de tâches",
    "Rechercher une tâche...": "Rechercher une tâche...",
    "Priorité:": "Priorité :",
    "Filtrer par priorité": "Filtrer par priorité",
    "Toutes": "Toutes",
    "Statut:": "Statut :",
    "Filtrer par statut": "Filtrer par statut",
    "Tous": "Tous",
    "Not started": "Non démarré",
    "In progress": "En cours",
    "Pending": "En attente",
    "Completed": "Terminé",
    "Cancelled": "Annulé",
    "Low": "Basse",
    "Medium": "Moyenne",
    "High": "Haute",
    "Critical": "Critique",
    "Ctrl+N": "Ctrl+N",
    "Ctrl+Shift+N": "Ctrl+Shift+N",
    "Ctrl+O": "Ctrl+O",
    "Ctrl+S": "Ctrl+S",
    "Ctrl+Shift+S": "Ctrl+Shift+S",
    "Ctrl+Q": "Ctrl+Q",
    "Del": "Suppr",
    "Ctrl+D": "Ctrl+D",
    "Ctrl+P": "Ctrl+P",
    "Ajouter une nouvelle tâche principale (Ctrl+N)": "Ajouter une nouvelle tâche principale (Ctrl+N)",
    "Ajouter tâche": "Ajouter tâche",
    "&Ajouter tâche": "&Ajouter tâche",
    "Ajouter une sous-tâche à la tâche sélectionnée (Ctrl+Shift+N)": "Ajouter une sous-tâche à la tâche sélectionnée (Ctrl+Shift+N)",
    "Ajouter sous-tâche": "Ajouter sous-tâche",
    "Supprimer la tâche sélectionnée (Del)": "Supprimer la tâche sélectionnée (Suppr)",
    "Marquer la tâche sélectionnée comme complétée (Ctrl+D)": "Marquer la tâche sélectionnée comme terminée (Ctrl+D)",
    "Marquer complétée": "Marquer complétée",
    "&Marquer complétée": "&Marquer complétée",
    "Sélectionnez une tâche pour voir les détails": "Sélectionnez une tâche pour voir les détails",
    "Barre d'outils": "Barre d'outils",
    "Créer un nouveau fichier de tâches": "Créer un nouveau fichier de tâches",
    "Ouvrir un fichier de tâches existant": "Ouvrir un fichier de tâches existant",
    "Enregistrer le fichier de tâches": "Enregistrer le fichier de tâches",
    "Enregistrer sous un nouveau nom": "Enregistrer sous un nouveau nom",
    "Quitter l'application": "Quitter l'application",
    "Ajouter une nouvelle tâche": "Ajouter une nouvelle tâche",
    "Ajouter une nouvelle sous-tâche": "Ajouter une nouvelle sous-tâche",
    "Supprimer la tâche sélectionnée": "Supprimer la tâche sélectionnée",
    "Marquer la tâche comme complétée": "Marquer la tâche comme terminée",
    "Promouvoir la sous-tâche en tâche principale": "Promouvoir la sous-tâche en tâche principale",
    "Afficher ou masquer les tâches terminées": "Afficher ou masquer les tâches terminées",
    "Afficher les informations sur l'application": "Afficher les informations sur l'application",
    "Titre:": "Titre :",
    "Description:": "Description :",
    "Date d'échéance:": "Date d'échéance :",
    "Priorité": "Priorité",
    "Statut": "Statut",
    "Appliquer": "Appliquer",
    "Nouvelle tâche": "Nouvelle tâche",
    "Nouvelle sous-tâche": "Nouvelle sous-tâche",
    "Ouvrir": "Ouvrir",
    "Enregistrer": "Enregistrer",
    "Supprimer": "Supprimer",
    "Marquer comme terminé": "Marquer comme terminé",
    "Promouvoir": "Promouvoir",
    "Title": "Titre",
    "Titre": "Titre",
    "Description": "Description",
    "Due Date": "Date d'échéance",
    "Priority": "Priorité",
    "Status": "Statut",
    "Supprimer la tâche": "Supprimer la tâche",
    "Êtes-vous sûr de vouloir supprimer cette tâche et toutes ses sous-tâches ?": "Êtes-vous sûr de vouloir supprimer cette tâche et toutes ses sous-tâches ?",
    "Cela marquera également toutes les sous-tâches comme terminées. Continuer ?": "Cela marquera également toutes les sous-tâches comme terminées. Continuer ?",
    "Promouvoir la tâche": "Promouvoir la tâche",
    "Êtes-vous sûr de vouloir promouvoir cette tâche au niveau racine ?": "Êtes-vous sûr de vouloir promouvoir cette tâche au niveau racine ?",
    "Enregistrer les modifications": "Enregistrer les modifications",
    "Le document contient des modifications non enregistrées. Voulez-vous les enregistrer avant de fermer ?": "Le document contient des modifications non enregistrées. Voulez-vous les enregistrer avant de fermer ?",
    "Ouvrir un fichier": "Ouvrir un fichier",
    "Enregistrer le fichier": "Enregistrer le fichier",
    "Fichiers JSON (*.json)": "Fichiers JSON (*.json)",
    "Tous les fichiers (*)": "Tous les fichiers (*)",
    "Erreur": "Erreur",
    "Impossible d'ouvrir le fichier en lecture.": "Impossible d'ouvrir le fichier en lecture.",
    "Impossible d'enregistrer le fichier.": "Impossible d'enregistrer le fichier.",
    "Prêt": "Prêt",
    "Total : %1 | Terminées : %2 | En attente : %3": "Total : %1 | Terminées : %2 | En attente : %3",
    "Sélectionné : %1": "Sélectionné : %1",
    "À propos de ToDoApp": "À propos de ToDoApp",
    "ToDoApp version 1.0": "ToDoApp version 1.0",
    "Une application de gestion de tâches": "Une application de gestion de tâches",
    "Développé avec Qt6": "Développé avec Qt6",
    "Auteurs :": "Auteurs :",
    "Crédits :": "Crédits :",
    "Licence : MIT": "Licence : MIT",
    "ToDoApp - %1": "ToDoApp - %1",
    "ToDoApp": "ToDoApp",
    # Nouvelles traductions pour les combos du TaskDetailWidget
    "Très faible": "Très faible",
    "Faible": "Faible",
    "Moyenne": "Moyenne",
    "Élevée": "Élevée",
    "Critique": "Critique",
    "Non démarrée": "Non démarrée",
    "En cours": "En cours",
    "Complétée": "Complétée",
    "Annulée": "Annulée",
    # Labels du TaskDetailWidget
    "Titre :": "Titre :",
    "Description :": "Description :",
    "Échéance :": "Échéance :",
    "Priorité :": "Priorité :",
    "Statut :": "Statut :",
    "Appliquer": "Appliquer",
}

# Traductions anglaises (noter que & est décodé automatiquement par le parser XML)
translations_en = {
    "ToDoApp - Gestionnaire de tâches": "ToDoApp - Task Manager",
    "Rechercher une tâche...": "Search for a task...",
    "Priorité:": "Priority:",
    "Filtrer par priorité": "Filter by priority",
    "Toutes": "All",
    "Statut:": "Status:",
    "Filtrer par statut": "Filter by status",
    "Tous": "All",
    "Not started": "Not started",
    "In progress": "In progress",
    "Pending": "Pending",
    "Completed": "Completed",
    "Cancelled": "Cancelled",
    "Low": "Low",
    "Medium": "Medium",
    "High": "High",
    "Critical": "Critical",
    "&Fichier": "&File",
    "&Édition": "&Edit",
    "&Affichage": "&View",
    "&Aide": "&Help",
    "&Langue": "&Language",
    "&Nouveau": "&New",
    "&Ouvrir...": "&Open...",
    "&Enregistrer": "&Save",
    "Enregistrer &sous...": "Save &As...",
    "&Quitter": "&Quit",
    "&Ajouter tâche": "&Add task",
    "Ajouter &sous-tâche": "Add &subtask",
    "&Supprimer": "&Delete",
    "&Marquer comme terminé": "&Mark as completed",
    "&Marquer complétée": "&Mark completed",
    "&Promouvoir": "&Promote",
    "&Afficher les tâches terminées": "&Show completed tasks",
    "À &propos": "&About",
    "Ctrl+N": "Ctrl+N",
    "Ctrl+Shift+N": "Ctrl+Shift+N",
    "Ctrl+O": "Ctrl+O",
    "Ctrl+S": "Ctrl+S",
    "Ctrl+Shift+S": "Ctrl+Shift+S",
    "Ctrl+Q": "Ctrl+Q",
    "Del": "Del",
    "Ctrl+D": "Ctrl+D",
    "Ctrl+P": "Ctrl+P",
    "Ajouter une nouvelle tâche principale (Ctrl+N)": "Add a new main task (Ctrl+N)",
    "Ajouter tâche": "Add task",
    "Ajouter une sous-tâche à la tâche sélectionnée (Ctrl+Shift+N)": "Add a subtask to the selected task (Ctrl+Shift+N)",
    "Ajouter sous-tâche": "Add subtask",
    "Supprimer la tâche sélectionnée (Del)": "Delete the selected task (Del)",
    "Marquer la tâche sélectionnée comme complétée (Ctrl+D)": "Mark the selected task as completed (Ctrl+D)",
    "Marquer complétée": "Mark completed",
    "Sélectionnez une tâche pour voir les détails": "Select a task to see details",
    "Barre d'outils": "Toolbar",
    "Créer un nouveau fichier de tâches": "Create a new task file",
    "Ouvrir un fichier de tâches existant": "Open an existing task file",
    "Enregistrer le fichier de tâches": "Save the task file",
    "Enregistrer sous un nouveau nom": "Save under a new name",
    "Quitter l'application": "Quit the application",
    "Ajouter une nouvelle tâche": "Add a new task",
    "Ajouter une nouvelle sous-tâche": "Add a new subtask",
    "Supprimer la tâche sélectionnée": "Delete the selected task",
    "Marquer la tâche comme complétée": "Mark the task as completed",
    "Promouvoir la sous-tâche en tâche principale": "Promote the subtask to main task",
    "Afficher ou masquer les tâches terminées": "Show or hide completed tasks",
    "Afficher les informations sur l'application": "Display information about the application",
    "Titre:": "Title:",
    "Description:": "Description:",
    "Date d'échéance:": "Due Date:",
    "Priorité": "Priority",
    "Statut": "Status",
    "Appliquer": "Apply",
    "Nouvelle tâche": "New task",
    "Nouvelle sous-tâche": "New subtask",
    "Ouvrir": "Open",
    "Enregistrer": "Save",
    "Supprimer": "Delete",
    "Marquer comme terminé": "Mark completed",
    "Promouvoir": "Promote",
    "Titre": "Title",
    "Description": "Description",
    "Date d'échéance": "Due Date",
    "Supprimer la tâche": "Delete Task",
    "Êtes-vous sûr de vouloir supprimer cette tâche et toutes ses sous-tâches ?": "Are you sure you want to delete this task and all its subtasks?",
    "Cela marquera également toutes les sous-tâches comme terminées. Continuer ?": "This will also mark all subtasks as completed. Continue?",
    "Promouvoir la tâche": "Promote Task",
    "Êtes-vous sûr de vouloir promouvoir cette tâche au niveau racine ?": "Are you sure you want to promote this task to root level?",
    "Enregistrer les modifications": "Save Changes",
    "Le document contient des modifications non enregistrées. Voulez-vous les enregistrer avant de fermer ?": "The document has unsaved changes. Do you want to save before closing?",
    "Ouvrir un fichier": "Open File",
    "Enregistrer le fichier": "Save File",
    "Fichiers JSON (*.json)": "JSON Files (*.json)",
    "Tous les fichiers (*)": "All Files (*)",
    "Erreur": "Error",
    "Impossible d'ouvrir le fichier en lecture.": "Could not open file for reading.",
    "Impossible d'enregistrer le fichier.": "Could not save file.",
    "Prêt": "Ready",
    "Total : %1 | Terminées : %2 | En attente : %3": "Total: %1 | Completed: %2 | Pending: %3",
    "Sélectionné : %1": "Selected: %1",
    "À propos de ToDoApp": "About ToDoApp",
    "ToDoApp version 1.0": "ToDoApp version 1.0",
    "Une application de gestion de tâches": "A task management application",
    "Développé avec Qt6": "Developed with Qt6",
    "Auteurs :": "Authors:",
    "Crédits :": "Credits:",
    "Licence : MIT": "License: MIT",
    "ToDoApp - %1": "ToDoApp - %1",
    "ToDoApp": "ToDoApp",
    # Nouvelles traductions pour les combos du TaskDetailWidget
    "Très faible": "Very Low",
    "Faible": "Low",
    "Moyenne": "Medium",
    "Élevée": "High",
    "Critique": "Critical",
    "Non démarrée": "Not Started",
    "En cours": "In Progress",
    "Complétée": "Completed",
    "Annulée": "Cancelled",
    # Traductions supplémentaires manquantes
    "Ajouter une sous-tâche": "Add a subtask",
    "Marquer comme complétée": "Mark as completed",
    "&Tout développer": "&Expand All",
    "Développer toutes les tâches": "Expand all tasks",
    "&Tout réduire": "&Collapse All",
    "Réduire toutes les tâches": "Collapse all tasks",
    "&Afficher les tâches complétées": "&Show completed tasks",
    "Afficher ou masquer les tâches complétées": "Show or hide completed tasks",
    "À &propos...": "&About...",
    "À propos de &Qt...": "About &Qt...",
    "À propos de Qt": "About Qt",
    "&Promouvoir au niveau parent": "&Promote to Parent Level",
    "Transformer la sous-tâche en tâche de même niveau que son parent": "Transform subtask to task at same level as parent",
    "&Français": "&French",
    "&English": "&English",
    "Tâche ajoutée : %1": "Task added: %1",
    "Tâche supprimée": "Task deleted",
    "ToDoApp - Nouveau fichier": "ToDoApp - New File",
    "Information": "Information",
    "Veuillez d'abord sélectionner une tâche parente.": "Please select a parent task first.",
    "Veuillez d'abord sélectionner une tâche à supprimer.": "Please select a task to delete first.",
    "Êtes-vous sûr de vouloir supprimer la tâche \"%1\" ?": "Are you sure you want to delete task \"%1\"?",
    "Confirmer la suppression": "Confirm Deletion",
    "Veuillez d'abord sélectionner une tâche.": "Please select a task first.",
    "Tâche et sous-tâches marquées comme en cours": "Task and subtasks marked as in progress",
    "Tâche et sous-tâches marquées comme complétées": "Task and subtasks marked as completed",
    "Veuillez d'abord sélectionner une sous-tâche à promouvoir.": "Please select a subtask to promote first.",
    "Cette tâche est déjà au niveau racine.": "This task is already at root level.",
    "Sous-tâche promue au niveau parent": "Subtask promoted to parent level",
    "Fichier enregistré : %1": "File saved: %1",
    "Échec de la sauvegarde": "Save failed",
    "Enregistrer sous": "Save As",
    "Fichiers JSON (*.json);;Tous les fichiers (*)": "JSON Files (*.json);;All Files (*)",
    "Les tâches actuelles seront perdues. Voulez-vous continuer ?": "Current tasks will be lost. Do you want to continue?",
    "Attention": "Warning",
    "Aucune tâche n'a pu être chargée depuis ce fichier.": "No tasks could be loaded from this file.",
    "Fichier chargé : %1 tâche(s)": "File loaded: %1 task(s)",
    "Nouveau fichier": "New File",
    "Nouveau fichier créé": "New file created",
    "Total: %1 tâche(s) | Complétées: %2 | En cours: %3": "Total: %1 task(s) | Completed: %2 | In Progress: %3",
    " | ⚠ En retard: %1": " | ⚠ Overdue: %1",
    " | Sélectionnée: \"%1\" - Priorité: %2 - Statut: %3": " | Selected: \"%1\" - Priority: %2 - Status: %3",
    " - Échéance: %1": " - Due: %1",
    "Catégorie": "Category",
    "Échéance :": "Due Date:",
    "Titre :": "Title:",
}


def fill_translations(ts_file, translations_dict):
    """Remplit les traductions manquantes dans un fichier .ts"""
    try:
        tree = ET.parse(ts_file)
        root = tree.getroot()
        
        filled = 0
        for context in root.findall('context'):
            for message in context.findall('message'):
                source = message.find('source')
                translation = message.find('translation')
                
                if source is not None and translation is not None:
                    source_text = source.text
                    is_unfinished = translation.get('type') == 'unfinished'
                    is_empty = not translation.text or translation.text.strip() == ''
                    
                    if (is_unfinished or is_empty) and source_text in translations_dict:
                        translation.text = translations_dict[source_text]
                        if 'type' in translation.attrib:
                            del translation.attrib['type']
                        filled += 1
                        src_short = source_text[:40] + "..." if len(source_text) > 40 else source_text
                        dst_short = translations_dict[source_text][:40] + "..." if len(translations_dict[source_text]) > 40 else translations_dict[source_text]
                        print(f"✓ '{src_short}' → '{dst_short}'")
        
        tree.write(ts_file, encoding='utf-8', xml_declaration=True)
        print(f"\n✅ {filled} traductions ajoutées dans {ts_file}")
        return filled
        
    except Exception as e:
        print(f"❌ Erreur : {e}", file=sys.stderr)
        return 0

if __name__ == "__main__":
    print("=== Traductions françaises ===")
    filled_fr = fill_translations("ToDoApp_fr.ts", translations_fr)
    
    print("\n=== Traductions anglaises ===")
    filled_en = fill_translations("ToDoApp_en_US.ts", translations_en)
    
    total = filled_fr + filled_en
    if total > 0:
        print(f"\n🎉 Total : {total} traductions ajoutées")
        print(f"\nMaintenant, exécutez :")
        print(f"  cd build/Desktop-Debug")
        print(f"  cmake --build . --target release_translations")
        print(f"  cp ToDoApp_*.qm ../..")
        print(f"  cmake --build .")
