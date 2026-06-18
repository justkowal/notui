#pragma once

#include "notui/Widget.h"
#include <vector>

namespace notui {

class FocusManager {
public:
    void rebuild(Widget& root);
    
    [[nodiscard]] auto focusedWidget() const -> Widget*;
    void set_focus(int index);
    void set_focus(Widget* widget);
    auto focus_next() -> bool;
    auto focus_prev() -> bool;
    void shift_focus_upstream_from(Widget* widget);
    auto handle_directional_focus(int key) -> bool;
    auto handleKeyboardInput(const ncinput& nc_input) -> bool;
    [[nodiscard]] static auto get_active_overlay(Widget* root) -> Widget*;

    [[nodiscard]] auto get_focusable_widgets() const -> const std::vector<Widget*>& { return focusable_widgets; }
    [[nodiscard]] auto get_focus_index() const -> int { return focus_idx; }

    void register_widget(Widget* widget);
    void unregister_widget(Widget* widget);
    void invalidate() { needs_rebuild = true; }

private:
    void collect_focusables(Widget* widget);

    std::vector<Widget*> focusable_widgets;
    int focus_idx = -1;
    bool needs_rebuild = true;
    Widget* cached_root = nullptr;
};

} 