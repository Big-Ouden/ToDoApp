# ToDoApp

A simple Qt-based To-Do list application demonstrating the use of Qt Widgets, CMake, UI forms, and translations.

## Features

* Add, remove, and manage tasks
* Qt Widgets user interface
* `.ui`-based design
* Translation system (`.ts` files)
* Optional documentation generation with Doxygen + AwesomeDoxygen

## Requirements

* Qt 5 or Qt 6
* CMake ≥ 3.16
* C++17
* (Optional) Doxygen for documentation generation

## Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Generate Documentation

```bash
cmake --build . --target doc_doxygen
```

Documentation output is located in:

```
doc/html/
```

## Authors

* **Simon Bélier & Wissal Jalfa**

## License

This project is released under the **GNU General Public License v2.0 (GPL-2.0)**.
See the `LICENSE` file for the full terms.

