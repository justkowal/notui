#include "notui/Container.h"
#include "notui/FocusManager.h"
#include <algorithm>
#include <ranges>

namespace notui {

void Container::add_child(std::shared_ptr<Widget> child) { 
    if (child != nullptr) {
        child->set_parent(this);
        children.push_back(std::move(child)); 
        if (focus_manager != nullptr) {
            focus_manager->invalidate();
        }
    }
}

void Container::remove_child(const std::shared_ptr<Widget>& child) {
    if (child == nullptr) {
        return;
    }
    auto iter = std::find(children.begin(), children.end(), child);
    if (iter != children.end()) {
        (*iter)->destroy_planes();
        (*iter)->set_parent(nullptr);
        children.erase(iter);
        if (focus_manager != nullptr) {
            focus_manager->invalidate();
        }
    }
}

auto Container::get_children() const -> const std::vector<std::shared_ptr<Widget>>& { 
    return children; 
}

void Container::destroy_planes() {
    for (auto& child : children) {
        if (child != nullptr) {
            child->destroy_planes();
        }
    }
    Widget::destroy_planes();
}

void Container::render() {
    style.apply(plane);
    ncplane_erase(plane);
    draw_box(style);
    for (auto& child : children) {
        if (child != nullptr && child->plane != nullptr) {
            child->render();
        }
    }
}

auto Container::contains_focus() -> bool {
    if (is_focused) {
        return true;
    }
    return std::ranges::any_of(children, [](const auto& child) {
        return child != nullptr && child->contains_focus();
    });
}

auto Container::get_widget_at(int pos_y, int pos_x) -> Widget* {
    if (plane == nullptr) {
        return nullptr;
    }
    int plane_y = ncplane_abs_y(plane);
    int plane_x = ncplane_abs_x(plane);
    if (pos_y >= plane_y && pos_y < plane_y + height && pos_x >= plane_x && pos_x < plane_x + width) {
        for (const auto& child : children | std::views::reverse) {
            if (child != nullptr) {
                Widget* hit = child->get_widget_at(pos_y, pos_x);
                if (hit != nullptr) {
                    return hit;
                }
            }
        }
        return this;
    }
    return nullptr;
}

} 
