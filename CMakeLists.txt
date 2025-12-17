cmake_minimum_required(VERSION 3.16)
project(ToDoApp VERSION 0.1 LANGUAGES CXX)

# Définir le type de build par défaut si non spécifié
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter les répertoires d'include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/models)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/widgets)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/git)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/utils)

# Configurer le répertoire des fichiers UI
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)

# ========================================
# Organisation des fichiers sources
# ========================================

# Core files
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "src/core/*.cpp")
file(GLOB CORE_HEADERS CONFIGURE_DEPENDS "src/core/*.h")

# Models
file(GLOB MODEL_SOURCES CONFIGURE_DEPENDS "src/models/*.cpp")
file(GLOB MODEL_HEADERS CONFIGURE_DEPENDS "src/models/*.h")

# Widgets
file(GLOB WIDGET_SOURCES CONFIGURE_DEPENDS "src/widgets/*.cpp")
file(GLOB WIDGET_HEADERS CONFIGURE_DEPENDS "src/widgets/*.h")

# Git integration
file(GLOB GIT_SOURCES CONFIGURE_DEPENDS "src/git/*.cpp")
file(GLOB GIT_HEADERS CONFIGURE_DEPENDS "src/git/*.h")

# Utils
file(GLOB UTIL_SOURCES CONFIGURE_DEPENDS "src/utils/*.cpp")
file(GLOB UTIL_HEADERS CONFIGURE_DEPENDS "src/utils/*.h")

# UI files
file(GLOB UI_FILES CONFIGURE_DEPENDS "ui/*.ui")

# Resources
file(GLOB RESOURCE_FILES CONFIGURE_DEPENDS "resources/*.qrc")
# Exclure translations.qrc de la liste automatique car on le génère dynamiquement
list(FILTER RESOURCE_FILES EXCLUDE REGEX "translations\\.qrc$")

# Fichiers de traduction
set(TS_FILES
    translations/ToDoApp_en_US.ts
    translations/ToDoApp_fr.ts
)

# Générer les fichiers .qm dans le répertoire de build
set(QM_FILES)
foreach(ts_file ${TS_FILES})
    get_filename_component(ts_name ${ts_file} NAME_WE)
    list(APPEND QM_FILES "${CMAKE_CURRENT_BINARY_DIR}/${ts_name}.qm")
endforeach()

# Compiler les .ts en .qm dans le répertoire de build
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

# Générer dynamiquement le fichier translations.qrc avec les bons chemins
set(TRANSLATIONS_QRC "${CMAKE_CURRENT_BINARY_DIR}/translations_generated.qrc")
file(WRITE ${TRANSLATIONS_QRC} "<RCC>\n    <qresource prefix=\"/i18n\">\n")
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename ${qm_file} NAME)
    file(APPEND ${TRANSLATIONS_QRC} "        <file alias=\"${qm_filename}\">${qm_file}</file>\n")
endforeach()
file(APPEND ${TRANSLATIONS_QRC} "    </qresource>\n</RCC>\n")

# Ajouter le fichier .qrc généré à la liste des ressources
list(APPEND RESOURCE_FILES ${TRANSLATIONS_QRC})

# S'assurer que les fichiers .qm sont générés avant la compilation
set_source_files_properties(${TRANSLATIONS_QRC} PROPERTIES OBJECT_DEPENDS "${QM_FILES}")

# Tous les fichiers sources
set(PROJECT_SOURCES
    ${CORE_SOURCES} ${CORE_HEADERS}
    ${MODEL_SOURCES} ${MODEL_HEADERS}
    ${WIDGET_SOURCES} ${WIDGET_HEADERS}
    ${GIT_SOURCES} ${GIT_HEADERS}
    ${UTIL_SOURCES} ${UTIL_HEADERS}
    ${UI_FILES}
    ${TS_FILES}
)

# ========================================
# Compilation de l'application
# ========================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ToDoApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${UI_FILES}
        ${RESOURCE_FILES}
        ${DOC_FILES}
        ${MISC_FILES}
    )
else()
    if(ANDROID)
        add_library(ToDoApp SHARED
            ${PROJECT_SOURCES}
            ${RESOURCE_FILES}
            ${DOC_FILES}
        )
    else()
        add_executable(ToDoApp
            ${PROJECT_SOURCES}
            ${RESOURCE_FILES}
            ${DOC_FILES}
        )
    endif()
endif()

target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::Network)

# ========================================
# Configuration de l'icône d'application
# ========================================
if(WIN32)
    # Windows: utiliser le fichier .rc
    target_sources(ToDoApp PRIVATE resources/app.rc)
elseif(APPLE)
    # macOS: configurer l'icône du bundle
    set(MACOSX_BUNDLE_ICON_FILE app_icon.icns)
    set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(ToDoApp PRIVATE ${APP_ICON_MACOSX})
else()
    # Linux: l'icône sera installée séparément
endif()

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.todoapp.ToDoApp)
endif()

set_target_properties(ToDoApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_BUNDLE_NAME "ToDoApp"
    MACOSX_BUNDLE_INFO_STRING "Gestionnaire de tâches hiérarchique"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Organisation visuelle dans Qt Creator
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})
source_group("Form Files" FILES ${UI_FILES})
source_group("Translation Files" FILES ${TS_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("Documentation" FILES ${DOC_FILES})
source_group("Miscellaneous Files" FILES ${MISC_FILES})

# ========================================
# 1. GÉNÉRATION DE LA DOCUMENTATION (Doxygen)
# ========================================
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.generated)

    # Configurer le Doxyfile pour qu'il pointe vers les bons chemins
    file(READ ${DOXYGEN_IN} DOXYFILE_CONTENT)
    string(REPLACE "INPUT                  = ."
                   "INPUT                  = ${CMAKE_CURRENT_SOURCE_DIR}"
                   DOXYFILE_CONTENT "${DOXYFILE_CONTENT}")
    string(REPLACE "OUTPUT_DIRECTORY       = doc"
                   "OUTPUT_DIRECTORY       = ${CMAKE_CURRENT_BINARY_DIR}/doc"
                   DOXYFILE_CONTENT "${DOXYFILE_CONTENT}")
    string(REPLACE "USE_MDFILE_AS_MAINPAGE = README.md"
                   "USE_MDFILE_AS_MAINPAGE = ${CMAKE_CURRENT_SOURCE_DIR}/README.md"
                   DOXYFILE_CONTENT "${DOXYFILE_CONTENT}")
    file(WRITE ${DOXYGEN_OUT} "${DOXYFILE_CONTENT}")

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Génération de la documentation avec Doxygen"
        VERBATIM
    )

    message(STATUS "Doxygen trouvé - cible 'doc' disponible")
else()
    message(WARNING "Doxygen n'a pas été trouvé - documentation non disponible")
endif()

# ========================================
# 2. GÉNÉRATION DU LISTING DU CODE
# ========================================

add_custom_target(listing
    COMMAND ${CMAKE_COMMAND} -E echo "Génération du listing du code source..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/listing
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_listing.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Création du listing du code source"
    VERBATIM
)

# Créer le script de génération de listing
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/generate_listing.cmake "
# Script de génération de listing

# Liste explicite des fichiers sources (mêmes fichiers que pour l'archive)
set(ALL_SOURCES
    ${HEADERS}
    ${SOURCES}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

set(LISTING_FILE \"${CMAKE_BINARY_DIR}/listing/code_listing.txt\")
file(WRITE \${LISTING_FILE} \"LISTING DU CODE SOURCE - ${PROJECT_NAME} v${PROJECT_VERSION}\\n\")
file(APPEND \${LISTING_FILE} \"========================================\\n\\n\")

foreach(source_file \${ALL_SOURCES})
    file(RELATIVE_PATH rel_path \"${CMAKE_CURRENT_SOURCE_DIR}\" \${source_file})
    file(APPEND \${LISTING_FILE} \"\\n\\n\")
    file(APPEND \${LISTING_FILE} \"================================================================================\\n\")
    file(APPEND \${LISTING_FILE} \"Fichier: \${rel_path}\\n\")
    file(APPEND \${LISTING_FILE} \"================================================================================\\n\\n\")
    file(READ \${source_file} FILE_CONTENT)
    file(APPEND \${LISTING_FILE} \"\${FILE_CONTENT}\")
endforeach()

message(STATUS \"Listing généré: \${LISTING_FILE}\")
")


# ========================================
# 3. GESTION DES TRADUCTIONS
# ========================================

# Mise à jour des fichiers .ts
add_custom_target(update_translations
    COMMAND Qt${QT_VERSION_MAJOR}::lupdate
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Mise à jour des fichiers de traduction (.ts)"
    VERBATIM
)

# Compilation des fichiers .ts en .qm
add_custom_target(release_translations
    COMMENT "Compilation des fichiers de traduction (.ts -> .qm)"
    VERBATIM
)

foreach(ts_file ${TS_FILES})
    get_filename_component(ts_name ${ts_file} NAME_WE)
    add_custom_command(
        TARGET release_translations POST_BUILD
        COMMAND Qt${QT_VERSION_MAJOR}::lrelease
            ${CMAKE_CURRENT_SOURCE_DIR}/${ts_file}
            -qm ${CMAKE_CURRENT_BINARY_DIR}/${ts_name}.qm
        COMMENT "Compilation de ${ts_file}"
    )
endforeach()

# Cible combinée pour les traductions
add_custom_target(translations
    COMMENT "Mise à jour et compilation des traductions"
)
add_dependencies(translations update_translations release_translations)

# ========================================
# 4. GÉNÉRATION D'ARCHIVE TRANSPORTABLE
# ========================================

set(ARCHIVE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

# Liste explicite des fichiers source à inclure
set(SOURCE_DIST_FILES
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${MODEL_SOURCES}
    ${MODEL_HEADERS}
    ${WIDGET_SOURCES}
    ${WIDGET_HEADERS}
    ${GIT_SOURCES}
    ${GIT_HEADERS}
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
    ${UI_FILES}
    ${TS_FILES}
    CMakeLists.txt
    README.md
    LICENSE
    exemple_taches.json
    update_translations.sh
    fill_translations.py
    Doxyfile
    generate_listing.cmake
)

# Ajouter tous les fichiers du répertoire resources (icônes, QSS, etc.)
file(GLOB_RECURSE RESOURCES_ALL RELATIVE ${CMAKE_SOURCE_DIR} "resources/*")
list(APPEND SOURCE_DIST_FILES ${RESOURCES_ALL})

# Ajouter tous les fichiers du répertoire docs
file(GLOB_RECURSE DOCS_ALL RELATIVE ${CMAKE_SOURCE_DIR} "docs/*.md")
list(APPEND SOURCE_DIST_FILES ${DOCS_ALL})

# Fichiers du rapport à inclure
set(RAPPORT_FILES
    rapport/rapport.tex
    rapport/rapport.pdf
    rapport/Makefile
    rapport/.gitignore
)

# Récupérer tous les fichiers d'images du rapport
file(GLOB RAPPORT_IMAGES RELATIVE ${CMAKE_SOURCE_DIR} "rapport/images/*")

# Combiner tous les fichiers pour l'archive
set(ALL_DIST_FILES
    ${SOURCE_DIST_FILES}
    ${RAPPORT_FILES}
    ${RAPPORT_IMAGES}
)

# On travaille depuis le répertoire source du projet
# pour que les chemins soient corrects dans l’archive
# (ex : ToDoApp/main.cpp)
add_custom_target(archive-tar
    COMMAND ${CMAKE_COMMAND} -E echo "Création de l'archive ${ARCHIVE_NAME}.tar.gz..."
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/src
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ui ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ui
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/translations ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/translations
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/misc ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/misc
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/rapport ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/rapport
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/CMakeLists.txt ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/exemple_taches.json ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/update_translations.sh ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/fill_translations.py ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Doxyfile ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/generate_listing.cmake ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/.gitignore ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E tar czf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz --format=gnutar ${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Archive TAR.GZ créée : ${ARCHIVE_NAME}.tar.gz (sources + rapport dans ${ARCHIVE_NAME}/)"
    VERBATIM
)

add_custom_target(archive-zip
    COMMAND ${CMAKE_COMMAND} -E echo "Création de l'archive ${ARCHIVE_NAME}.zip..."
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/src
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ui ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/ui
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/translations ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/translations
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/misc ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/misc
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/rapport ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/rapport
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/CMakeLists.txt ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/exemple_taches.json ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/update_translations.sh ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/fill_translations.py ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Doxyfile ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/generate_listing.cmake ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/.gitignore ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.zip --format=zip ${ARCHIVE_NAME}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Archive ZIP créée : ${ARCHIVE_NAME}.zip (sources + rapport dans ${ARCHIVE_NAME}/)"
    VERBATIM
)

# Cible générale
add_custom_target(archive
    DEPENDS archive-zip archive-tar
    COMMENT "Génération des archives du projet"
)


# ========================================
# 5. CIBLE "ALL-IN-ONE" - GÉNÉRATION COMPLÈTE
# ========================================
add_custom_target(deliverable
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target translations
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target doc
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target listing
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target archive
    COMMENT "Génération complète: documentation + listing + traductions + archive"
    VERBATIM
)

# ========================================
# Installation
# ========================================
include(GNUInstallDirs)
install(TARGETS ToDoApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Installer les fichiers de traduction
install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_BINDIR}/translations
)

# Installer l'icône sur Linux
if(UNIX AND NOT APPLE)
    install(FILES resources/app_icon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME todoapp.svg
    )
endif()

# ========================================
# CPack - Génération de packages
# ========================================
set(CPACK_PACKAGE_NAME "ToDoApp")
set(CPACK_PACKAGE_VENDOR "ToDoApp Development Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gestionnaire de tâches hiérarchique multiplateforme")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ToDoApp")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "ToDoApp")
    set(CPACK_NSIS_PACKAGE_NAME "ToDoApp")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.ico")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "todoapp@example.com")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6widgets6")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

include(CPack)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ToDoApp)
endif()

# ========================================
# Résumé des cibles disponibles
# ========================================
message(STATUS "==============================================")
message(STATUS "Cibles disponibles:")
message(STATUS "  - ToDoApp          : Compilation de l'application")
message(STATUS "  - doc              : Génération de la documentation")
message(STATUS "  - listing          : Génération du listing du code")
message(STATUS "  - translations     : Mise à jour et compilation des traductions")
message(STATUS "  - update_translations : Mise à jour des .ts")
message(STATUS "  - release_translations : Compilation des .ts en .qm")
message(STATUS "  - archive          : Création d'une archive du projet")
message(STATUS "  - archive-zip      : Archive au format ZIP")
message(STATUS "  - archive-tar      : Archive au format TAR.GZ")
message(STATUS "  - deliverable      : TOUT générer (doc + listing + trad + archive)")
message(STATUS "==============================================")
