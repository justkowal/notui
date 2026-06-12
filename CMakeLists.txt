# notui terminal library
# consumed via add_subdirectory from parent

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

# build interface include path
target_include_directories(notui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(notui PUBLIC PkgConfig::NOTCURSES)

# exclude from clang-tidy
set_target_properties(notui PROPERTIES CXX_CLANG_TIDY "")