#include "notui/FocusManager.h"
#include "notui/Container.h"
#include <algorithm>
#include <cmath>

namespace notui {

namespace {
auto find_active_overlay(Widget* widget) -> Widget* {
    if (widget == nullptr) {
        return nullptr;
    }
    if (widget->is_overlay) {
        return widget;
    }
    if (auto* container = dynamic_cast<Container*>(widget)) {
        for (const auto& child : container->get_children()) {
            Widget* overlay = find_active_overlay(child.get());
            if (overlay != nullptr) {
                return overlay;
            }
        }
    }
    return nullptr;
}
} // namespace

void FocusManager::rebuild(Widget& root) {
    Widget* prev_focused = focusedWidget();
    focusable_widgets.clear();
    
    Widget* overlay = find_active_overlay(&root);
    if (overlay != nullptr) {
        collect_focusables(overlay);
    } else {
        collect_focusables(&root);
    }

    auto iter = std::find(focusable_widgets.begin(), focusable_widgets.end(), prev_focused);
    if (iter != focusable_widgets.end()) {
        focus_idx = static_cast<int>(std::distance(focusable_widgets.begin(), iter));
    } else {
        focus_idx = -1;
    }

    if (focus_idx < 0 && !focusable_widgets.empty()) {
        set_focus(0);
    } else if (focus_idx >= 0 && focus_idx < static_cast<int>(focusable_widgets.size())) {
        focusable_widgets[focus_idx]->on_focus();
    }
}

auto FocusManager::get_active_overlay(Widget* root) -> Widget* {
    return find_active_overlay(root);
}

auto FocusManager::focusedWidget() const -> Widget* {
    if (focus_idx >= 0 && focus_idx < static_cast<int>(focusable_widgets.size())) {
        return focusable_widgets[focus_idx];
    }
    return nullptr;
}

void FocusManager::set_focus(int index) {
    if (focusable_widgets.empty()) {
        return;
    }
    if (focus_idx == index) {
        return;
    }
    if (focus_idx >= 0 && focus_idx < static_cast<int>(focusable_widgets.size())) {
        focusable_widgets[focus_idx]->on_blur();
    }
    
    focus_idx = index;
    if (focus_idx >= 0 && focus_idx < static_cast<int>(focusable_widgets.size())) {
        focusable_widgets[focus_idx]->on_focus();
    }
}

void FocusManager::set_focus(Widget* widget) {
    auto iter = std::find(focusable_widgets.begin(), focusable_widgets.end(), widget);
    if (iter != focusable_widgets.end()) {
        set_focus(static_cast<int>(std::distance(focusable_widgets.begin(), iter)));
    }
}

auto FocusManager::focus_next() -> bool {
    if (focusable_widgets.empty()) {
        return false;
    }
    set_focus((focus_idx + 1) % static_cast<int>(focusable_widgets.size()));
    return true;
}

auto FocusManager::focus_prev() -> bool {
    if (focusable_widgets.empty()) {
        return false;
    }
    int size = static_cast<int>(focusable_widgets.size());
    set_focus((focus_idx - 1 + size) % size);
    return true;
}

void FocusManager::shift_focus_upstream_from(Widget* widget) {
    if (focusable_widgets.empty()) {
        return;
    }
    auto iter = std::find(focusable_widgets.begin(), focusable_widgets.end(), widget);
    if (iter == focusable_widgets.end()) {
        return;
    }
    int idx = static_cast<int>(std::distance(focusable_widgets.begin(), iter));
    int size = static_cast<int>(focusable_widgets.size());
    bool found = false;
    for (int i = 1; i <= size; ++i) {
        int prev_idx = (idx - i + size) % size;
        Widget* candidate = focusable_widgets[prev_idx];
        if (candidate != widget && !candidate->disabled) {
            set_focus(candidate);
            found = true;
            break;
        }
    }
    if (!found) {
        set_focus(-1);
    }
}

auto FocusManager::handle_directional_focus(int key) -> bool { // NOLINT(readability-function-cognitive-complexity)
    if (focusable_widgets.empty() || focus_idx < 0) {
        return false;
    }
    Widget* current = focusable_widgets[focus_idx];
    if (current->plane == nullptr) {
        return false;
    }

    int cur_y = ncplane_abs_y(current->plane);
    int cur_x = ncplane_abs_x(current->plane);
    
    int best_idx = -1;
    int min_dist = 999999;

    for (size_t i = 0; i < focusable_widgets.size(); ++i) {
        if (static_cast<int>(i) == focus_idx) {
            continue;
        }
        Widget* widget = focusable_widgets[i];
        if (widget->plane == nullptr) {
            continue;
        }

        int w_y = ncplane_abs_y(widget->plane);
        int w_x = ncplane_abs_x(widget->plane);
        
        int diff_y = w_y - cur_y;
        int diff_x = w_x - cur_x;
        
        bool valid = false;
        if (key == NCKEY_UP && diff_y < 0) {
            valid = true;
        }
        if (key == NCKEY_DOWN && diff_y > 0) {
            valid = true;
        }
        if (key == NCKEY_LEFT && diff_x < 0) {
            valid = true;
        }
        if (key == NCKEY_RIGHT && diff_x > 0) {
            valid = true;
        }
        
        if (valid) {
            int dist = 0;
            if (key == NCKEY_UP || key == NCKEY_DOWN) {
                dist = std::abs(diff_y) * 10 + std::abs(diff_x) * 50; 
            } else {
                dist = std::abs(diff_x) * 10 + std::abs(diff_y) * 50;
            }
            
            if (dist < min_dist) {
                min_dist = dist;
                best_idx = static_cast<int>(i);
            }
        }
    }
    
    if (best_idx != -1) {
        set_focus(best_idx);
        return true;
    }
    return false;
}

auto FocusManager::handleKeyboardInput(const ncinput& nc_input) -> bool {
    if (nc_input.id == NCKEY_TAB && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
        const bool shift_pressed = nc_input.shift || ((nc_input.modifiers & NCKEY_MOD_SHIFT) != 0);
        if (shift_pressed) {
            return focus_prev();
        } else {
            return focus_next();
        }
    }
    if (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN) {
        if (nc_input.id == NCKEY_UP || nc_input.id == NCKEY_DOWN || nc_input.id == NCKEY_LEFT || nc_input.id == NCKEY_RIGHT) {
            return handle_directional_focus(static_cast<int>(nc_input.id));
        }
    }
    return false;
}

void FocusManager::collect_focusables(Widget* widget) {
    if (widget == nullptr) {
        return;
    }
    widget->focus_manager = this;
    if (widget->focusable && !widget->disabled) {
        focusable_widgets.push_back(widget);
    }
    if (auto* container = dynamic_cast<Container*>(widget)) {
        for (const auto& child : container->get_children()) {
            collect_focusables(child.get());
        }
    }
}

} // namespace notui