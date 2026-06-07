#include "notui/HBox.h"
#include <algorithm>

namespace notui {

HBox::HBox(struct ncplane* parent, int y, int x, int height, int width)
    : Widget(parent, y, x, height, width) {}

auto HBox::addChild(std::unique_ptr<Widget> child) -> void {
    if (child) {
        child->setFocusManager(focus_manager_);
    }
    children_.push_back(std::move(child));
    recalculateLayout();
}

auto HBox::setFocusManager(FocusManager* manager) -> void {
    Widget::setFocusManager(manager);
    for (auto& child : children_) {
        child->setFocusManager(manager);
    }
}

auto HBox::resizeAndMove(int y, int x, int height, int width) -> void {
    Widget::resizeAndMove(y, x, height, width);
    recalculateLayout();
}

auto HBox::recalculateLayout() -> void {
    if (children_.empty()) return;

    int total_fixed_width = 0;
    int total_percent_width = 0;
    int expand_count = 0;

    std::vector<int> calculated_widths(children_.size(), 0);

    // PASS 1: Calculate Fixed and Percent widths
    for (size_t i = 0; i < children_.size(); ++i) {
        const auto& policy = children_[i]->getWidthPolicy();
        
        if (policy.mode == SizeMode::Fixed) {
            calculated_widths[i] = policy.value;
            total_fixed_width += policy.value;
        } 
        else if (policy.mode == SizeMode::Percent) {
            int w = (width_ * policy.value) / 100;
            calculated_widths[i] = w;
            total_percent_width += w;
        } 
        else if (policy.mode == SizeMode::Expand) {
            expand_count++;
        }
    }

    // PASS 2: Calculate remaining space for 'Expand' widgets
    int remaining_space = std::max(0, width_ - total_fixed_width - total_percent_width);
    int expand_width = expand_count > 0 ? (remaining_space / expand_count) : 0;

    // PASS 3: Apply the geometry to the children across the X axis
    int current_x = 0;
    for (size_t i = 0; i < children_.size(); ++i) {
        int final_width = calculated_widths[i];
        
        if (children_[i]->getWidthPolicy().mode == SizeMode::Expand) {
            final_width = expand_width;
        }

        // HBox forces children to stretch to its full height, aligning side-by-side
        children_[i]->resizeAndMove(0, current_x, height_, final_width);
        current_x += final_width;
    }
}

auto HBox::render() -> void {
    ncplane_erase(plane_);
    for (auto& child : children_) {
        child->render();
    }
}

auto HBox::handleInput(const ncinput& input) -> bool {
    // Bubble input. First child to consume it wins.
    for (auto& child : children_) {
        if (child->handleInput(input)) {
            return true;
        }
    }
    return false;
}

auto HBox::collectFocusable(std::vector<Widget*>& focusables) -> void {
    for (auto& child : children_) {
        child->collectFocusable(focusables);
    }
}

} // namespace notui