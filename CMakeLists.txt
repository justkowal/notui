# ---------------------------------------------------------------------------
# notui — terminal UI widget library (git submodule)
# ---------------------------------------------------------------------------
# Note: cmake_minimum_required / project() are intentionally omitted here.
# This file is consumed via add_subdirectory() from the parent project,
# which already owns the version policy and language setup.
# ---------------------------------------------------------------------------

add_library(notui STATIC
    src/Widget.cpp
    src/Container.cpp
    src/VBox.cpp
    src/HBox.cpp
    src/SplitBox.cpp
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

# BUILD_INTERFACE: consumers that link via add_subdirectory() get this path.
# INSTALL_INTERFACE would be set if we ever package/install the library.
target_include_directories(notui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(notui PUBLIC PkgConfig::NOTCURSES)

# Exclude this vendored library from clang-tidy analysis
set_target_properties(notui PROPERTIES CXX_CLANG_TIDY "")