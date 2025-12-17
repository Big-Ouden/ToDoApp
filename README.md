# ToDoApp

Application de gestion de tâches hiérarchiques développée avec Qt Widgets.

## Description

ToDoApp est une application de gestion de tâches permettant de créer, organiser et suivre des tâches avec support des sous-tâches, des priorités, des statuts et des étiquettes. L'application offre une interface graphique intuitive avec support multilingue (français/anglais), export PDF et recherche avancée.

## Fonctionnalités principales

* Gestion hiérarchique des tâches et sous-tâches
* **Intégration Git/Issues** : Synchronisation avec GitHub/GitLab/Gitea
  * Mode Personnel : tâches quotidiennes classiques
  * Mode Git : suivi d'issues de projets externes
  * Multi-repos : gérez plusieurs dépôts simultanément
  * Liaison tâches ↔ issues pour workflow hybride
* Système de priorités (Low, Medium, High, Critical)
* Suivi du statut (Not started, In progress, Completed, Cancelled)
* Étiquettes (tags) pour catégoriser les tâches
* Pièces jointes pour lier des fichiers aux tâches
* Description au format Markdown avec aperçu en temps réel
* Recherche avancée avec préfixes (tag:, priority:, status:, date:)
* Export PDF avec templates personnalisables
* Interface multilingue (français, anglais)
* Sauvegarde/chargement au format JSON
* Interface responsive avec panneau de détails

## Prérequis

* Qt 5.15 ou Qt 6.2+ (composants : Widgets, Core, PrintSupport, LinguistTools, Network)
* CMake >= 3.16
* Compilateur C++17 compatible (GCC, Clang, MSVC)
* Doxygen >= 1.9 (optionnel, pour générer la documentation)

## Compilation

### Linux / macOS

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
./ToDoApp
```

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
.\Release\ToDoApp.exe
```

## Options en ligne de commande

L'application supporte plusieurs options CLI pour le débogage :

```bash
# Afficher l'aide
./ToDoApp --help

# Afficher la version
./ToDoApp --version

# Activer le mode debug (affiche les opérations importantes)
./ToDoApp --debug
./ToDoApp -d

# Activer le mode verbeux (affiche tous les détails)
./ToDoApp --verbose
```

Le mode debug affiche :
- Chargement des traductions
- Opérations de sauvegarde/chargement de fichiers
- Suppression de tâches
- Nombre de tâches chargées

Le mode verbeux affiche en plus :
- Détails de chaque tâche chargée/supprimée
- Informations système au démarrage

### Qt Creator

1. Ouvrir le fichier `CMakeLists.txt`
2. Configurer le projet avec le kit Qt approprié
3. Compiler (Ctrl+B) et exécuter (Ctrl+R)

## Cibles CMake disponibles

### Application principale

```bash
make                    # Compiler l'application
make clean              # Nettoyer les fichiers de build
```

### Documentation

```bash
make doc               # Générer la documentation Doxygen (HTML)
```

Documentation générée dans : `build/doc/html/index.html`

### Code listing

```bash
make listing           # Générer un listing complet du code source
```

Listing généré dans : `build/listing/code_listing.txt`

### Traductions

```bash
make update_translations     # Extraire les chaînes traduisibles du code
make release_translations    # Compiler les fichiers .ts en .qm
make translations           # update + release
```

#### Scripts de traduction

**`update_translations.sh`**
Script shell qui automatise la mise à jour et compilation des traductions :
- Extrait toutes les chaînes `tr()` du code C++ vers les fichiers `.ts`
- Compile les fichiers `.ts` (XML) en fichiers `.qm` (binaires)
- Copie les `.qm` générés dans le répertoire racine pour l'exécution

**`fill_translations.py`**
Script Python qui remplit automatiquement les traductions manquantes :
- Contient un dictionnaire prédéfini de traductions FR/EN
- Parse les fichiers `.ts` (XML) et remplit les balises `<translation>` vides
- Supprime les marqueurs `type="unfinished"` pour les traductions complétées
- Utile pour traduire rapidement les chaînes communes de l'interface

**Workflow typique :**
```bash
# 1. Développeur ajoute du code avec tr("Nouveau texte")
# 2. Extraire les nouvelles chaînes
./update_translations.sh

# 3. Remplir automatiquement les traductions connues
cd translations/
python3 ../fill_translations.py

# 4. Éditer manuellement les traductions restantes avec Qt Linguist
linguist ToDoApp_fr.ts ToDoApp_en_US.ts

# 5. Recompiler les traductions
cd ..
./update_translations.sh
```

### Archives

```bash
make archive-zip       # Créer une archive ZIP du projet
make archive-tar       # Créer une archive TAR.GZ
make archive          # Créer les deux archives
```

### Livrable complet

```bash
make deliverable      # Générer doc + listing + traductions + archives
```

## Structure du projet

```
ToDoApp/
├── CMakeLists.txt              # Configuration CMake
├── Doxyfile                    # Configuration Doxygen
├── README.md                   # Ce fichier
├── LICENSE                     # Licence GPLv2
│
├── src/                        # Code source
│   ├── core/                   # Point d'entrée
│   │   └── main.cpp            # Main de l'application
│   │
│   ├── models/                 # Modèles de données
│   │   ├── task.{h,cpp}        # Modèle de tâche
│   │   ├── taskmodel.{h,cpp}   # Modèle arborescent Qt
│   │   ├── taskfilterproxymodel.{h,cpp} # Filtrage et recherche
│   │   ├── taskitemdelegate.{h,cpp} # Délégué d'affichage
│   │   ├── priority.h          # Enum des priorités
│   │   ├── status.h            # Enum des statuts
│   │   └── category.{h,cpp}    # Gestion des catégories
│   │
│   ├── widgets/                # Widgets Qt
│   │   ├── mainwindow.{h,cpp}  # Fenêtre principale
│   │   ├── taskdetailwidget.{h,cpp} # Panneau de détails
│   │   ├── statisticswidget.{h,cpp} # Statistiques
│   │   ├── kanbanview.{h,cpp}  # Vue Kanban
│   │   ├── heatmapwidget.{h,cpp} # Heatmap d'activité
│   │   ├── focusmodedialog.{h,cpp} # Mode focus
│   │   ├── pomodorotimer.{h,cpp} # Timer Pomodoro
│   │   ├── chartswidget.{h,cpp} # Graphiques
│   │   ├── timelinewidget.{h,cpp} # Timeline
│   │   └── burndownwidget.{h,cpp} # Burndown chart
│   │
│   ├── git/                    # Intégration Git/Issues
│   │   ├── gitrepository.{h,cpp} # Modèle de dépôt
│   │   ├── repositorymanager.{h,cpp} # Gestionnaire multi-dépôts
│   │   ├── githubconnector.{h,cpp} # API GitHub
│   │   ├── gitlabconnector.{h,cpp} # API GitLab
│   │   ├── gitissuetask.{h,cpp} # Tâche liée à une issue
│   │   ├── gitprojectwidget.{h,cpp} # Widget de projet Git
│   │   └── gitsyncdialog.{h,cpp} # Dialogue de synchronisation
│   │
│   └── utils/                  # Utilitaires
│       ├── persistencemanager.{h,cpp} # Sauvegarde/chargement JSON
│       ├── pdfexporttemplate.{h,cpp} # Export PDF
│       ├── themesmanager.{h,cpp} # Gestion des thèmes
│       ├── streakscalculator.{h,cpp} # Calcul de séries
│       ├── undocommands.{h,cpp} # Système Undo/Redo
│       └── debug.h              # Macros de débogage
│
├── ui/                         # Fichiers d'interface Qt Designer
│   ├── mainwindow.ui
│   ├── taskdetailwidget.ui
│   ├── gitprojectwidget.ui
│   └── statisticswidget.ui
│
├── resources/                  # Ressources
│   ├── ressource.qrc           # Fichier de ressources Qt
│   ├── icons/                  # Icônes PNG
│   └── themes/                 # Thèmes QSS
│       ├── light.qss           # Thème clair
│       ├── dark.qss            # Thème sombre
│       └── warm.qss            # Thème chaud (crème/orange)
│
├── translations/               # Fichiers de traduction
│   ├── ToDoApp_fr.ts           # Français
│   └── ToDoApp_en_US.ts        # Anglais
│
├── docs/                       # Documentation
│   ├── GIT_INTEGRATION.md      # Guide technique Git
│   ├── GIT_USER_GUIDE.md       # Guide utilisateur Git
│   ├── GUIDE_UTILISATEUR.md    # Guide général
│   ├── DOCUMENTATION_COMPLETE.md # Documentation Doxygen
│   └── *.md                    # Autres documentations
│
├── misc/                       # Fichiers divers
│   ├── modele_graphe.uxf       # Diagramme UML
│   └── todo.md                 # Notes de développement
│
└── build/                      # Dossier de compilation (généré)
    ├── ToDoApp                 # Exécutable
    ├── doc/html/               # Documentation générée
    └── listing/                # Listing du code
```

## Utilisation

### Créer une tâche

1. Cliquer sur "Nouvelle tâche" ou utiliser le raccourci Ctrl+N
2. Remplir les informations dans le panneau de droite
3. Cliquer sur "Appliquer" pour enregistrer

### Ajouter une sous-tâche

1. Sélectionner une tâche parente
2. Cliquer sur "Ajouter sous-tâche" ou utiliser Ctrl+Shift+N
3. Remplir les informations de la sous-tâche

### Recherche avancée

Utiliser les préfixes dans le champ de recherche :

* `tag:urgent` - Chercher par étiquette
* `priority:high` - Filtrer par priorité
* `status:completed` - Filtrer par statut
* `date:2025-12` - Filtrer par date d'échéance

### Export PDF

1. Menu Fichier > Exporter en PDF
2. Choisir un template (Default, Minimal, Detailed, Colorful)
3. Sélectionner l'emplacement et enregistrer

### Thèmes visuels

ToDoApp propose 3 thèmes personnalisables :

1. **Thème Light (Clair)** - Fond blanc, idéal pour le jour
2. **Thème Dark (Sombre)** - Fond noir, réduit la fatigue oculaire
3. **Thème Warm (Chaud)** - Tons crème/orange, filtre la lumière bleue

**Raccourcis clavier :**
- **Ctrl+W** : Activer le thème Warm
- Menu → Thèmes pour basculer entre les 3 thèmes

**Personnalisation :**
Les thèmes sont modifiables via les fichiers QSS dans `resources/themes/`:
- `light.qss` - Thème clair
- `dark.qss` - Thème sombre  
- `warm.qss` - Thème chaud

### Raccourcis clavier

**Navigation :**
- **Ctrl+T** : Basculer en Mode Personnel (Todo classique)
- **Ctrl+G** : Basculer en Mode Git (Issues)

**Édition :**
- **Ctrl+N** : Nouvelle tâche
- **Ctrl+Shift+N** : Ajouter sous-tâche
- **Delete** : Supprimer tâche sélectionnée

**Thèmes :**
- **Ctrl+W** : Thème Warm (chaud)

## Intégration Git/Issues

ToDoApp peut se connecter à **GitHub, GitLab et Gitea** pour synchroniser les issues de vos projets.

### Démarrage rapide

1. **Basculer en Mode Git** : Menu → Vue → Mode Git/Issues
2. **Ajouter un dépôt** : Bouton "Ajouter"
   - Plateforme : GitHub ou GitLab
   - Owner/Repo : `torvalds/linux`
   - Token : Votre Personal Access Token
3. **Synchroniser** : Cliquez sur "Synchroniser"
4. **Créer une tâche liée** : Sélectionnez une issue → "Créer tâche liée"
5. **Revenir en Mode Personnel** : La tâche apparaît dans votre liste

### Génération des tokens

**GitHub** :
- https://github.com/settings/tokens → Generate token → Scope `repo`

**GitLab** :
- https://gitlab.com/-/profile/personal_access_tokens → Scope `api`

### Multi-repos

Vous pouvez configurer plusieurs dépôts (GitHub + GitLab) et basculer entre eux via la liste déroulante.

### Documentation complète

Consultez **GIT_USER_GUIDE.md** pour :
- Configuration détaillée
- Workflow recommandés
- Sécurité et bonnes pratiques
- Dépannage
- Exemples concrets

Consultez **GIT_INTEGRATION.md** pour :
- Architecture technique
- API documentation
- Contribution au code

## Licence

Ce projet est distribué sous licence GNU General Public License v2.0 (GPLv2).
Voir le fichier LICENSE pour plus de détails.

## Auteurs

* Simon Bélier
* Wissal Jalfa

## Contribution

Ce projet a été développé dans le cadre d'un projet académique.

## Documentation technique

La documentation complète générée par Doxygen est disponible après compilation :

```bash
make doc
xdg-open build/doc/html/index.html  # Linux
open build/doc/html/index.html      # macOS
start build\doc\html\index.html     # Windows
```

