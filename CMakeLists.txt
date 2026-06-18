
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

target_include_directories(notui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(MINGW)
    link_directories(${NOTCURSES_STATIC_LIBRARY_DIRS})
    target_include_directories(notui PUBLIC ${NOTCURSES_INCLUDE_DIRS})
    set(NOTUI_MINGW_LIBS ${NOTCURSES_STATIC_LIBRARIES})
    list(REMOVE_ITEM NOTUI_MINGW_LIBS "notcurses")
    target_link_libraries(notui PUBLIC ${NOTUI_MINGW_LIBS} ws2_32 secur32)
else()
    target_link_libraries(notui PUBLIC PkgConfig::NOTCURSES)
endif()

target_compile_options(notui PRIVATE
    $<$<CONFIG:Debug>:
        -Wall
        -Wextra
        -Wshadow
    >
    $<$<CONFIG:Release>:
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wformat=2
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Werror
    >
)

set_target_properties(notui PROPERTIES CXX_CLANG_TIDY "")
