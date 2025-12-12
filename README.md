# ToDoApp

Application de gestion de tâches hiérarchiques développée avec Qt Widgets.

## Description

ToDoApp est une application de gestion de tâches permettant de créer, organiser et suivre des tâches avec support des sous-tâches, des priorités, des statuts et des étiquettes. L'application offre une interface graphique intuitive avec support multilingue (français/anglais), export PDF et recherche avancée.

## Fonctionnalités principales

* Gestion hiérarchique des tâches et sous-tâches
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

* Qt 5.15 ou Qt 6.2+ (composants : Widgets, Core, PrintSupport, LinguistTools)
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
├── main.cpp                    # Point d'entrée
├── mainwindow.{h,cpp,ui}       # Fenêtre principale
├── task.{h,cpp}                # Modèle de tâche
├── taskmodel.{h,cpp}           # Modèle arborescent Qt
├── taskfilterproxymodel.{h,cpp} # Filtrage et recherche
├── taskdetailwidget.{h,cpp,ui} # Panneau de détails
├── persistencemanager.{h,cpp}  # Sauvegarde/chargement JSON
├── pdfexporttemplate.{h,cpp}   # Templates export PDF
├── priority.h                  # Enum des priorités
├── status.h                    # Enum des statuts
├── ToDoApp_fr.ts               # Traduction française
├── ToDoApp_en_US.ts            # Traduction anglaise
└── build/                      # Dossier de compilation (généré)
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

