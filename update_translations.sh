#!/bin/bash
# Script pour mettre à jour et compiler les traductions

echo "Mise à jour des fichiers .ts..."
cd "$(dirname "$0")"
cd build/Desktop-Debug
cmake --build . --target update_translations

echo ""
echo "Compilation des fichiers .qm..."
cmake --build . --target release_translations

echo ""
echo "Copie des .qm dans le répertoire source..."
cp ToDoApp_*.qm ../..

echo ""
echo "Fichiers .qm générés:"
ls -lh ../../*.qm

echo ""
echo "✅ Traductions mises à jour avec succès!"
