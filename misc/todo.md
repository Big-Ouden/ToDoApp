# TODO LIST - ToDoApp Qt Project

**Projet:** Application de gestion de tâches (ToDo List)  
**Deadline:** 19 décembre 2025  
**Validation modèle:** 20 novembre 2025

---

## PHASE 1 : MODÈLE DE DONNÉES (Priorité: CRITIQUE)

### 1.1 Énumérations et structures de base

- [ ] **Créer les enums dans `task.h`**
  - `enum class Priority` : Low, Medium, High, Critical
  - `enum class Status` : NotStarted, InProgress, Completed, Cancelled

### 1.2 Classe Task

- [ ] **Créer `task.h` et `task.cpp`**
  - Hériter de `QObject` pour utiliser signaux/slots
  - Ajouter `Q_OBJECT` macro en début de classe
  - Déclarer les attributs privés (m_id, m_title, m_description, m_dueDate, m_priority, m_status, m_subtasks, m_parentTask)
  
- [ ] **Implémenter les getters/setters**
  - Tous les setters doivent émettre un signal de notification
  - Utiliser `Q_PROPERTY` pour exposer les propriétés au Meta-Object System
  
- [ ] **Déclarer les signaux nécessaires**
  - Signal pour chaque propriété modifiée (titleChanged, descriptionChanged, etc.)
  - Signal `taskModified()` générique
  - Signaux pour gestion sous-tâches (subtaskAdded, subtaskRemoved)
  
- [ ] **Gérer la hiérarchie parent/enfant**
  - Maintenir pointeur `m_parentTask`
  - Liste `m_subtasks` (QList<Task*>)
  - Méthodes: addSubtask(), removeSubtask(), hasSubtasks()
  
- [ ] **Méthodes utilitaires**
  - `isCompleted()` : vérifier si status == Completed
  - `isOverdue()` : comparer dueDate avec QDate::currentDate()
  - `completionPercentage()` : calculer % basé sur sous-tâches complétées
  
- [ ] **Documentation Doxygen**
  - `@brief` pour chaque méthode
  - `@param` pour les paramètres
  - `@return` pour les valeurs de retour
  - `@signal` pour documenter les signaux

### 1.3 Classe Category

- [ ] **Créer `category.h` et `category.cpp`**
  - Hériter de `QObject`
  - Attributs: m_id (QUuid), m_name (QString), m_color (QColor), m_tasks (QList<Task*>)
  
- [ ] **Propriétés Q_PROPERTY**
  - name, color (avec signaux nameChanged, colorChanged)
  
- [ ] **Gestion des tâches**
  - addTask(), removeTask(), containsTask()
  - **ATTENTION:** Category ne possède PAS les Task (pas de delete), juste des références
  
- [ ] **Documentation Doxygen complète**

### 1.4 Classe TaskModel (Le cœur du MVC)

- [ ] **Créer `taskmodel.h` et `taskmodel.cpp`**
  - **Choix crucial:** Hériter de `QAbstractItemModel` (arbre Task/SubTask) OU `QAbstractListModel` (liste plate)
  - Recommandation: `QAbstractItemModel` pour supporter la hiérarchie
  
- [ ] **Attributs du modèle**
  - `QList<Task*> m_rootTasks` : liste des tâches principales (le modèle possède ces objets)
  - Optionnel: `QList<Category*> m_categories`
  
- [ ] **Implémenter les méthodes obligatoires de QAbstractItemModel**
  - `rowCount(const QModelIndex &parent)` : nombre d'enfants d'un nœud
  - `columnCount(const QModelIndex &parent)` : nombre de colonnes (titre, date, priorité, statut, etc.)
  - `data(const QModelIndex &index, int role)` : données à afficher (Qt::DisplayRole, Qt::EditRole, Qt::DecorationRole)
  - `setData(const QModelIndex &index, const QVariant &value, int role)` : modifier une donnée
  - `index(int row, int column, const QModelIndex &parent)` : créer un QModelIndex
  - `parent(const QModelIndex &child)` : obtenir le parent d'un index
  - `flags(const QModelIndex &index)` : Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
  
- [ ] **Méthodes de gestion des tâches**
  - `insertTask(Task *task, const QModelIndex &parent = QModelIndex())`
    - Appeler `beginInsertRows()` AVANT insertion
    - Ajouter la tâche dans m_rootTasks ou comme sous-tâche
    - Appeler `endInsertRows()` APRÈS insertion
  - `removeTask(const QModelIndex &index)`
    - Appeler `beginRemoveRows()` / `endRemoveRows()`
    - delete la Task (libérer mémoire)
  - `getTask(const QModelIndex &index)` : récupérer pointeur Task depuis index
  
- [ ] **Signaux personnalisés** (en plus de ceux de QAbstractItemModel)
  - `taskAdded(Task *task)`
  - `taskRemoved(Task *task)`
  - `taskUpdated(Task *task)`
  
- [ ] **Connecter les signaux Task → TaskModel**
  - Quand une Task émet `taskModified()`, le modèle doit émettre `dataChanged()`
  - Utiliser `connect()` dans `insertTask()`
  
- [ ] **Support du Drag & Drop (optionnel mais valorisé)**
  - Surcharger `supportedDropActions()`, `mimeTypes()`, `mimeData()`, `dropMimeData()`
  
- [ ] **Documentation Doxygen complète**

### 1.5 Graphe de classes UML

- [ ] **Créer le diagramme de classes**
  - Outils suggérés: draw.io, PlantUML, Dia
  - Montrer: Task, Category, TaskModel, leurs relations
  - Indiquer: héritage (QObject, QAbstractItemModel), composition, agrégation
  - Signaler les signaux/slots principaux
  
- [ ] **Valider avec l'enseignant le 20/11/2025**

---

## 🖥️ PHASE 2 : INTERFACE GRAPHIQUE (Priorité: HAUTE)

### 2.1 Structure principale (mainwindow.ui avec Qt Designer)

- [ ] **Ouvrir `mainwindow.ui` dans Qt Designer**
  
- [ ] **Widget central**
  - Supprimer le QWidget vide actuel
  - Ajouter un `QSplitter` (orientation horizontale) comme widget central
  - **Partie gauche du splitter:**
    - Choisir `QTreeView` (recommandé pour hiérarchie) OU `QListView`
    - Nommer: `taskTreeView` ou `taskListView`
  - **Partie droite du splitter:**
    - `QWidget` (conteneur pour détails)
    - Y ajouter un `QVBoxLayout`
    - Ce widget sera remplacé plus tard par votre `TaskDetailWidget` custom

- [ ] **Menus (QMenuBar)**
  - Menu "Fichier"
    - Action: "Nouveau" (Ctrl+N) → slot `onNewFile()`
    - Action: "Ouvrir..." (Ctrl+O) → slot `onOpenFile()`
    - Action: "Enregistrer" (Ctrl+S) → slot `onSaveFile()`
    - Action: "Enregistrer sous..." → slot `onSaveFileAs()`
    - Séparateur
    - Action: "Quitter" (Ctrl+Q) → slot `QApplication::quit()`
  - Menu "Édition"
    - Action: "Ajouter tâche" (Ctrl+T) → slot `onAddTask()`
    - Action: "Ajouter sous-tâche" (Ctrl+Shift+T) → slot `onAddSubtask()`
    - Action: "Supprimer" (Del) → slot `onDeleteTask()`
    - Séparateur
    - Action: "Marquer comme terminée" → slot `onMarkCompleted()`
  - Menu "Aide"
    - Action: "À propos" → slot `onAbout()`
    - Action: "À propos de Qt" → slot `QApplication::aboutQt()`

- [ ] **Barre d'outils (QToolBar)**
  - Ajouter les actions principales: Ajouter, Supprimer, Enregistrer
  - Utiliser les icônes (icons/add.png, etc.)
  - Ajouter un `QLineEdit` comme widget dans la toolbar pour la recherche

- [ ] **Barre d'état (QStatusBar)**
  - Déjà présente dans mainwindow.ui
  - Sera utilisée pour afficher: "X tâches | Y complétées | Z en retard"

### 2.2 Widget personnalisé pour les détails

- [ ] **Créer `taskdetailwidget.h` et `taskdetailwidget.cpp`**
  - Hériter de `QWidget`
  - Ce widget servira à afficher ET éditer une tâche
  
- [ ] **Créer `taskdetailwidget.ui` dans Qt Designer**
  - Utiliser un `QFormLayout` principal
  - Ajouter les champs:
    - QLineEdit pour titre
    - QTextEdit pour description
    - QDateEdit pour date d'échéance (avec calendrier popup)
    - QComboBox pour priorité (peupler avec enum Priority)
    - QComboBox pour statut (peupler avec enum Status)
    - QProgressBar pour afficher le % de complétion des sous-tâches
  - Ajouter des boutons: "Sauvegarder", "Annuler"
  
- [ ] **Méthodes de TaskDetailWidget**
  - `setTask(Task *task)` : afficher les infos d'une tâche
  - `clearTask()` : vider tous les champs
  - Slots internes pour gérer les modifications
  
- [ ] **Signaux de TaskDetailWidget**
  - `taskModified()` : émis quand l'utilisateur modifie un champ
  
- [ ] **Validation des entrées**
  - Utiliser `QRegularExpressionValidator` pour le titre (non vide)
  - QDateEdit valide automatiquement les dates
  
- [ ] **Intégrer TaskDetailWidget dans MainWindow**
  - Dans `mainwindow.cpp` (constructeur), promouvoir le QWidget de droite en `TaskDetailWidget`
  - OU créer dynamiquement: `m_detailWidget = new TaskDetailWidget(this);`

### 2.3 Connexions signaux/slots dans MainWindow

- [ ] **Dans `mainwindow.h`**
  - Ajouter attributs privés:
    - `TaskModel *m_taskModel;`
    - `TaskDetailWidget *m_detailWidget;`
    - `QString m_currentFilePath;`
  
- [ ] **Dans `mainwindow.cpp` (constructeur)**
  - Instancier le modèle: `m_taskModel = new TaskModel(this);`
  - Connecter le modèle à la vue: `ui->taskTreeView->setModel(m_taskModel);`
  - Connecter signal de sélection:

    ```cpp
    connect(ui->taskTreeView->selectionModel(), 
            &QItemSelectionModel::currentChanged,
            this, 
            &MainWindow::onTaskSelectionChanged);
    ```
  
- [ ] **Slots privés à déclarer dans `mainwindow.h`**

  ```cpp
  private slots:
      void onTaskSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
      void onAddTask();
      void onAddSubtask();
      void onDeleteTask();
      void onMarkCompleted();
      void onSaveFile();
      void onSaveFileAs();
      void onOpenFile();
      void onNewFile();
      void onSearchTextChanged(const QString &text);
      void onAbout();
      void updateStatusBar();
  ```
  
- [ ] **Implémenter chaque slot**
  - `onAddTask()`: Créer une Task, l'insérer dans m_taskModel
  - `onTaskSelectionChanged()`: Récupérer la Task via getTask(index), appeler m_detailWidget->setTask()
  - `onDeleteTask()`: Confirmer avec QMessageBox, appeler m_taskModel->removeTask()
  - `onSearchTextChanged()`: Filtrer la vue (voir Phase 5)
  - `updateStatusBar()`: Parcourir toutes les tâches, compter, afficher dans statusBar()

### 2.4 Gestion des icônes et ressources

- [ ] **Compléter `ressource.qrc`**
  - Ajouter toutes vos icônes (delete.png, save.png, open.png, etc.)
  - Utiliser un prefix: `<qresource prefix="/icons">`
  
- [ ] **Utiliser les icônes dans les actions**
  - `QIcon icon(":/icons/add.png");`
  - `action->setIcon(icon);`

---

## 🔗 PHASE 3 : PERSISTANCE DES DONNÉES (Priorité: HAUTE)

### 3.1 Classe PersistenceManager

- [ ] **Créer `persistencemanager.h` et `persistencemanager.cpp`**
  - Classe statique (ou singleton) avec méthodes statiques
  - Pas besoin d'hériter de QObject
  
- [ ] **Méthode `saveToJson()`**
  - Signature: `static bool saveToJson(const QString &filePath, const QList<Task*> &tasks);`
  - Utiliser `QJsonDocument`, `QJsonArray`, `QJsonObject`
  - Parcourir toutes les tâches, sérialiser récursivement
  - Chaque Task → QJsonObject avec clés: "id", "title", "description", "dueDate", "priority", "status", "subtasks"
  - Gérer les sous-tâches récursivement
  - Sauver avec `QFile` et `QTextStream` ou `QFile::write()`
  
- [ ] **Méthode `loadFromJson()`**
  - Signature: `static QList<Task*> loadFromJson(const QString &filePath);`
  - Ouvrir le fichier avec `QFile`
  - Parser avec `QJsonDocument::fromJson()`
  - Désérialiser chaque objet JSON → reconstruire des Task*
  - **Attention:** Reconstruire la hiérarchie parent/enfant correctement
  - Gérer les erreurs: fichier introuvable, JSON invalide
  
- [ ] **Méthodes XML (optionnel si temps)**
  - `saveToXml()` avec `QDomDocument`, `QDomElement`
  - `loadFromXml()` avec parsing DOM
  
- [ ] **Gestion des erreurs**
  - Retourner false en cas d'échec
  - Afficher des messages d'erreur avec `qWarning()` ou `qDebug()`

### 3.2 Intégration dans MainWindow

- [ ] **Implémenter `onSaveFile()`**
  - Si `m_currentFilePath` est vide → appeler `onSaveFileAs()`
  - Sinon: `PersistenceManager::saveToJson(m_currentFilePath, m_taskModel->rootTasks());`
  - Afficher message de succès dans la barre d'état
  
- [ ] **Implémenter `onSaveFileAs()`**
  - Utiliser `QFileDialog::getSaveFileName()` avec filtre "*.json"
  - Sauvegarder puis stocker le chemin dans `m_currentFilePath`
  
- [ ] **Implémenter `onOpenFile()`**
  - Utiliser `QFileDialog::getOpenFileName()`
  - Appeler `PersistenceManager::loadFromJson()`
  - Vider le modèle actuel: `m_taskModel->clear()` (à implémenter)
  - Insérer toutes les tâches chargées dans le modèle
  
- [ ] **Sauvegarder automatiquement à la fermeture**
  - Surcharger `MainWindow::closeEvent(QCloseEvent *event)`
  - Demander confirmation si non sauvegardé (flag `m_isModified`)

---

## PHASE 4 : RECHERCHE ET FILTRAGE (Priorité: MOYENNE)

### 4.1 QSortFilterProxyModel

- [ ] **Créer `taskfilterproxymodel.h` et `taskfilterproxymodel.cpp`**
  - Hériter de `QSortFilterProxyModel`
  - Cet objet s'intercale entre TaskModel et la Vue
  
- [ ] **Surcharger `filterAcceptsRow()`**
  - Récupérer la Task depuis le sourceModel
  - Vérifier si elle correspond aux critères de recherche
  - Critères possibles: texte dans titre/description, priorité, statut, date
  
- [ ] **Ajouter des setters pour les filtres**
  - `setSearchText(const QString &text)` : stocker dans un attribut, appeler `invalidateFilter()`
  - `setPriorityFilter(Priority p)` : idem
  - `setStatusFilter(Status s)` : idem
  
- [ ] **Dans MainWindow**
  - Créer: `m_proxyModel = new TaskFilterProxyModel(this);`
  - `m_proxyModel->setSourceModel(m_taskModel);`
  - `ui->taskTreeView->setModel(m_proxyModel);` (au lieu de m_taskModel directement)
  
- [ ] **Connecter le champ de recherche**
  - `connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);`
  - Dans `onSearchTextChanged()`: `m_proxyModel->setSearchText(text);`

### 4.2 Interface de recherche avancée (optionnel)

- [ ] **Ajouter des ComboBox pour filtrer par priorité/statut**
  - Dans la toolbar ou un dock widget
  - Connecter à des slots qui appellent setFilter() sur le proxy

---

## PHASE 5 : FONCTIONNALITÉS AVANCÉES (Priorité: BASSE)

### 5.1 Édition in-place avec QStyledItemDelegate

- [ ] **Créer `taskitemdelegate.h` et `taskitemdelegate.cpp`**
  - Hériter de `QStyledItemDelegate`
  
- [ ] **Surcharger `createEditor()`**
  - Selon la colonne, retourner un éditeur approprié:
    - Colonne titre: QLineEdit
    - Colonne priorité: QComboBox avec les valeurs de l'enum
    - Colonne date: QDateEdit
  
- [ ] **Surcharger `setEditorData()` et `setModelData()`**
  - Transférer données modèle ↔ éditeur
  
- [ ] **Surcharger `paint()` (optionnel)**
  - Personnaliser le rendu: icône de priorité, couleur selon statut, etc.
  
- [ ] **Installer le délégué**
  - `ui->taskTreeView->setItemDelegate(new TaskItemDelegate(this));`

### 5.2 Gestion des catégories

- [ ] **Ajouter un QDockWidget pour les catégories**
  - Dans MainWindow, ajouter un dock à gauche
  - Y placer une QListView avec un modèle de catégories
  
- [ ] **Créer CategoryModel**
  - Hériter de `QAbstractListModel`
  - Gérer une `QList<Category*>`
  
- [ ] **Assigner tâches aux catégories**
  - Ajouter un QComboBox dans TaskDetailWidget pour choisir la catégorie
  - Connecter à `Category::addTask()` / `removeTask()`

### 5.3 Tri des tâches

- [ ] **Activer le tri dans la vue**
  - `ui->taskTreeView->setSortingEnabled(true);`
  
- [ ] **Implémenter le tri dans TaskFilterProxyModel**
  - Surcharger `lessThan()`
  - Comparer selon la colonne (date, priorité, titre, etc.)

### 5.4 Notifications et couleurs

- [ ] **Colorer les tâches en retard**
  - Dans `TaskModel::data()`, si `role == Qt::ForegroundRole`
  - Retourner `QBrush(Qt::red)` si la tâche est en retard
  
- [ ] **Icônes de priorité**
  - Dans `TaskModel::data()`, si `role == Qt::DecorationRole`
  - Retourner un QIcon selon la priorité

---

## PHASE 6 : DOCUMENTATION ET POLISH (Priorité: HAUTE avant rendu)

### 6.1 Documentation Doxygen

- [ ] **Installer Doxygen** (si pas déjà fait)
  - Site officiel: doxygen.nl
  
- [ ] **Créer un fichier `Doxyfile`**
  - Commande: `doxygen -g Doxyfile`
  - Configurer: `PROJECT_NAME`, `INPUT` (dossiers source), `OUTPUT_DIRECTORY`
  
- [ ] **Documenter TOUTES les classes**
  - Format: `/** @brief ... */` avant chaque classe/méthode
  - `@param nom_param Description`
  - `@return Description`
  - `@signal Description` pour les signaux
  
- [ ] **Générer la documentation HTML**
  - Commande: `doxygen Doxyfile`
  - Vérifier le rendu dans `html/index.html`

### 6.2 README et rapport

- [ ] **Créer `README.md`**
  - Titre et description du projet
  - Instructions de compilation (qmake, cmake)
  - Fonctionnalités implémentées
  - Captures d'écran
  
- [ ] **Rédiger le rapport final**
  - Graphe de classes complet (modèle + vues + contrôleurs)
  - Description de chaque fonctionnalité avec captures d'écran
  - Tableau: Fonctionnalité → Comment l'utiliser (Menu? Bouton? Raccourci?)
  - Difficultés rencontrées et solutions

### 6.3 Tests et débogage

- [ ] **Tester toutes les fonctionnalités**
  - Créer/Modifier/Supprimer tâches
  - Sous-tâches (hiérarchie)
  - Sauvegarde/Chargement JSON
  - Recherche et filtrage
  - Édition dans la vue
  
- [ ] **Vérifier les fuites mémoire**
  - Utiliser valgrind (Linux) ou Instruments (macOS)
  - S'assurer que chaque `new` a son `delete`
  - Les Task sont détruits quand supprimés du modèle
  
- [ ] **Gestion des cas limites**
  - Ouvrir un fichier corrompu
  - Supprimer une tâche avec sous-tâches
  - Champs vides

### 6.4 Améliorations UI/UX

- [ ] **Icônes pour toutes les actions**
  - Menu, toolbar, boutons
  
- [ ] **Messages de confirmation**
  - `QMessageBox::question()` avant suppression
  - Demander de sauvegarder avant quitter si modifié
  
- [ ] **Raccourcis clavier**
  - Déjà définis dans les QAction (Ctrl+N, Ctrl+S, etc.)
  - Ajouter F2 pour renommer, Del pour supprimer
  
- [ ] **Style CSS (optionnel)**
  - `qApp->setStyleSheet(...)` pour personnaliser l'apparence
  - Ou utiliser les styles natifs de Qt

---

## RESSOURCES UTILES

### Classes Qt à connaître

**Modèle:**

- `QAbstractItemModel` : base pour modèles hiérarchiques
- `QAbstractListModel` : base pour listes plates
- `QModelIndex` : représente un élément dans le modèle
- `QObject` : base pour signaux/slots

**Vues:**

- `QTreeView` : vue arborescente (recommandé pour hiérarchie)
- `QListView` : vue liste
- `QTableView` : vue tableau

**Délégués:**

- `QStyledItemDelegate` : pour personnaliser rendu et édition

**Filtrage/Tri:**

- `QSortFilterProxyModel` : pour filtrer et trier un modèle

**Persistance:**

- `QJsonDocument`, `QJsonObject`, `QJsonArray` : manipulation JSON
- `QFile`, `QTextStream` : lecture/écriture fichiers
- `QDomDocument`, `QDomElement` : manipulation XML

**Widgets:**

- `QMainWindow` : fenêtre principale
- `QSplitter` : séparateur redimensionnable
- `QDockWidget` : fenêtre ancrée
- `QLineEdit`, `QTextEdit`, `QDateEdit`, `QComboBox` : champs de saisie

**Validation:**

- `QValidator`, `QRegularExpressionValidator` : validation de saisie

**Dialogues:**

- `QFileDialog` : ouvrir/enregistrer fichiers
- `QMessageBox` : messages d'alerte/confirmation

### Signaux/Slots importants

**QAbstractItemModel:**

- Signaux: `dataChanged()`, `rowsInserted()`, `rowsRemoved()`
- À émettre manuellement après modifications

**QItemSelectionModel:**

- Signal: `currentChanged(QModelIndex, QModelIndex)`
- Pour détecter changement de sélection

**QLineEdit:**

- Signal: `textChanged(QString)`
- Pour détecter saisie en temps réel

**Task (custom):**

- Tous vos signaux: `titleChanged()`, `statusChanged()`, etc.

### Concepts clés à maîtriser

- **Architecture MVC:** Séparation Modèle / Vue / Contrôleur
- **Meta-Object System:** Q_OBJECT, signaux/slots, Q_PROPERTY
- **Ownership en Qt:** Qui possède et détruit les objets (parent/enfant)
- **QModelIndex:** Différence entre index valide/invalide, internalPointer()
- **beginInsertRows/endInsertRows:** TOUJOURS appeler avant/après modification du modèle

---

## PIÈGES À ÉVITER

1. **Ne PAS utiliser de boîtes de dialogue pour l'édition**
   - Éditer directement dans la vue (délégué) ou panneau de droite

2. **Ne PAS oublier beginInsertRows() / endInsertRows()**
   - Sinon la vue ne se met pas à jour

3. **Ne PAS mélanger modèle et vue**
   - TaskModel ne doit PAS inclure de QWidget
   - Les signaux Qt permettent la communication

4. **Bien gérer la mémoire**
   - Les Task* dans le modèle doivent être `delete` quand supprimés
   - Utiliser le système parent/enfant de Qt quand possible

5. **Tester régulièrement**
   - Compiler et tester après chaque fonctionnalité
   - Ne pas tout coder d'un coup

6. **Documenter au fur et à mesure**
   - Ne pas attendre la fin pour Doxygen

---

## CRITÈRES DE NOTATION (Rappel)

- **Modèle de données** (30%)
  - Hiérarchie d'héritage
  - Composition
  - Signaux/Slots
  - Type de modèle Qt

- **Fonctionnalités** (40%)
  - Affichage structure (liste/arbre)
  - Affichage détails
  - Édition
  - Création/Suppression
  - Recherche/Filtrage
  - Sauvegarde/Chargement

- **Interface graphique** (20%)
  - Layouts
  - Composants d'édition
  - Édition in-place
  - Synchronisation dynamique
  - Validation
  - Barre d'état

- **Documentation** (10%)
  - Doxygen complet
  - Rapport clair
  - README
