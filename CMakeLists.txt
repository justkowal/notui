cmake_minimum_required(VERSION 3.14)
project(notui VERSION 0.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NOTCURSES REQUIRED IMPORTED_TARGET notcurses)

add_library(notui STATIC
    src/Widget.cpp
    src/Container.cpp
    src/VBox.cpp
    src/HBox.cpp
    src/Spacer.cpp
    src/Label.cpp
    src/Button.cpp
    src/ScrollArea.cpp
    src/Dropdown.cpp
    src/Modal.cpp
    src/FocusManager.cpp
    src/Compositor.cpp
    src/Checkbox.cpp
    src/TextArea.cpp
    src/Canvas.cpp
)

target_include_directories(notui PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(notui PUBLIC PkgConfig::NOTCURSES)

set_target_properties(notui PROPERTIES CXX_CLANG_TIDY "")