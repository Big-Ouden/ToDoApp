cmake_minimum_required(VERSION 3.16)
project(ToDoApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# Documentation Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Génération de la documentation avec Doxygen"
        VERBATIM
    )
endif()

# Organisation des fichiers par catégorie
set(HEADERS
    mainwindow.h
)

set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(UI_FILES
    mainwindow.ui
)

set(TS_FILES
    ToDoApp_en_US.ts
)

set(RESOURCE_FILES
    # resources.qrc
)

set(DOC_FILES
    README.md
    Doxyfile
)

# Regroupement de tous les fichiers sources
set(PROJECT_SOURCES
    ${HEADERS}
    ${SOURCES}
    ${UI_FILES}
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ToDoApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCE_FILES}
        ${DOC_FILES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
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
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(ToDoApp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ToDoApp)
endif()

set_target_properties(ToDoApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Organisation visuelle dans Qt Creator avec source_group
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})
source_group("Form Files" FILES ${UI_FILES})
source_group("Translation Files" FILES ${TS_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("Documentation" FILES ${DOC_FILES})

include(GNUInstallDirs)
install(TARGETS ToDoApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ToDoApp)
endif()
