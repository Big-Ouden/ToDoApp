# ToDoApp

Application de gestion de tâches (To-Do list) développée avec Qt Widgets dans le cadre d'un projet académique.

## Fonctionnalités

* Ajout, suppression et gestion de tâches
* Interface graphique Qt Widgets
* Interface conçue avec Qt Designer (fichiers `.ui`)
* Support multilingue (système de traduction avec fichiers `.ts`)
* Génération automatique de documentation avec Doxygen
* Génération de listing du code source
* Création d'archives pour la livraison du projet

## Prérequis

* Qt 5 ou Qt 6 (avec les composants Widgets et LinguistTools)
* CMake ≥ 3.16
* Compilateur compatible C++17
* (Optionnel) Doxygen pour la génération de documentation

## Compilation

### Compilation standard

```bash
# Créer le dossier de build
mkdir build
cd build

# Configurer le projet
cmake ..

# Compiler l'application
cmake --build .

# Lancer l'application
./ToDoApp
```

### Avec Qt Creator

1. Ouvrir `CMakeLists.txt` dans Qt Creator
2. Configurer le projet avec le kit souhaité
3. Compiler et exécuter le projet (Ctrl+R)

## Cibles CMake disponibles

Le projet fournit plusieurs cibles utiles pour le développement et la livraison :

### Application

```bash
# Compiler l'application principale
cmake --build . --target ToDoApp
```

### Documentation

```bash
# Générer la documentation HTML avec Doxygen
cmake --build . --target doc
```

La documentation est générée dans : `build/doc/html/index.html`

### Listing du code

```bash
# Générer un listing complet de tout le code source
cmake --build . --target listing
```

Le listing est généré dans : `build/listing/code_listing.txt`

### Traductions

```bash
# Mettre à jour les fichiers de traduction (.ts) depuis le code source
cmake --build . --target update_translations

# Compiler les fichiers de traduction (.ts -> .qm)
cmake --build . --target release_translations

# Mettre à jour ET compiler les traductions
cmake --build . --target translations
```

### Création d'archive

```bash
# Créer une archive ZIP du projet
cmake --build . --target archive-zip

# Créer une archive TAR.GZ du projet
cmake --build . --target archive-tar

# Créer les deux archives
cmake --build . --target archive
```

Les archives sont créées dans le dossier `build/` avec le nom : `ToDoApp-<version>-src.zip`

### Livrable complet

```bash
# Générer TOUT d'un coup :
# - Documentation
# - Listing du code
# - Traductions
# - Archive du projet
cmake --build . --target deliverable
```

**Cette commande est idéale pour générer tous les livrables du projet en une seule fois !** 🎯

## Structure du projet

```
ToDoApp/
├── CMakeLists.txt          # Configuration CMake
├── Doxyfile             # Fichier de configuration Doxygen
├── README.md               # Ce fichier
├── main.cpp                # Point d'entrée de l'application
├── mainwindow.h/cpp        # Implémentation de la fenêtre principale
├── mainwindow.ui           # Interface Qt Designer
├── ToDoApp_en_US.ts        # Traduction anglaise
├── ToDoApp_fr_FR.ts        # Traduction française
└── build/                  # Dossier de compilation (généré)
    ├── ToDoApp             # Exécutable
    ├── doc/                # Documentation générée
    ├── listing/            # Listing du code généré
    └── *.zip               # Archives générées
```

## Ajout de nouveaux fichiers sources

Lors de l'ajout de nouveaux fichiers `.cpp`, `.h` ou `.ui`, ils seront automatiquement détectés grâce à l'option `CONFIGURE_DEPENDS` dans CMakeLists.txt. Il suffit de recompiler le projet.

## Traductions

### Ajouter une nouvelle langue

1. Ajouter le nouveau fichier `.ts` dans la variable `TS_FILES` du `CMakeLists.txt` :
   ```cmake
   set(TS_FILES
       ToDoApp_en_US.ts
       ToDoApp_fr_FR.ts
       ToDoApp_es_ES.ts  # Espagnol
   )
   ```

2. Générer et mettre à jour les traductions :
   ```bash
   cmake --build . --target update_translations
   ```

3. Ouvrir le fichier `.ts` avec Qt Linguist pour traduire les chaînes de caractères

4. Compiler les traductions :
   ```bash
   cmake --build . --target release_translations
   ```

## Personnalisation

### Exclure des fichiers de la documentation

Éditer `Doxyfile` et modifier la ligne `EXCLUDE_PATTERNS` :

```doxyfile
EXCLUDE_PATTERNS = */build/* \
                   */mon_dossier_exclu/*
```

### Exclure des fichiers de l'archive

Éditer `CMakeLists.txt` et modifier la variable `EXCLUDE_PATTERNS` :

```cmake
set(EXCLUDE_PATTERNS
    ".git/*"
    "build*/*"
    "*.user"
    # Ajoutez vos patterns ici
)
```

## Résolution de problèmes

### "No files to be processed" lors de la génération de documentation

Supprimez le dossier `build/` et reconfigurez :

```bash
rm -rf build
mkdir build
cd build
cmake ..
```

### Outils de traduction introuvables

Installer Qt Linguist :

```bash
# Ubuntu/Debian
sudo apt install qttools5-dev-tools

# Fedora
sudo dnf install qt5-linguist

# macOS (avec Homebrew)
brew install qt
```

### Doxygen introuvable

Installer Doxygen :

```bash
# Ubuntu/Debian
sudo apt install doxygen

# Fedora
sudo dnf install doxygen

# macOS (avec Homebrew)
brew install doxygen
```

## Rendu du projet

Pour générer tous les livrables nécessaires au rendu du projet :

```bash
cd build
cmake --build . --target deliverable
```

Cette commande génère automatiquement :
- La documentation dans `build/doc/html/`
- Le listing du code dans `build/listing/code_listing.txt`
- Les fichiers de traduction compilés (`.qm`)
- L'archive du projet `ToDoApp-0.1-src.zip`

## Auteurs

* **Simon Bélier**
* **Wissal Jalfa**

**Projet académique** - Cours de Langages et Outils pour l'Algorithmique Avancée (LAOA)

## Licence

Ce projet est distribué sous la **GNU General Public License v2.0 (GPL-2.0)**.

Voir le fichier `LICENSE` pour les détails complets.

## Remerciements

* Projet Qt pour l'excellent framework Qt
* Équipe Doxygen pour les outils de génération de documentation
* Développeurs CMake pour le système de build
