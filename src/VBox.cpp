#include "notui/VBox.h"
#include <algorithm>

namespace notui {

VBox::VBox(struct ncplane* parent, int y, int x, int height, int width)
    : Widget(parent, y, x, height, width) {}

auto VBox::addChild(std::unique_ptr<Widget> child) -> void {
    if (child) {
        child->setFocusManager(focus_manager_);
    }
    children_.push_back(std::move(child));
    recalculateLayout();
}

auto VBox::setFocusManager(FocusManager* manager) -> void {
    Widget::setFocusManager(manager);
    for (auto& child : children_) {
        child->setFocusManager(manager);
    }
}

auto VBox::resizeAndMove(int y, int x, int height, int width) -> void {
    Widget::resizeAndMove(y, x, height, width);
    recalculateLayout();
}

auto VBox::recalculateLayout() -> void {
    if (children_.empty()) return;

    int total_fixed_height = 0;
    int total_percent_height = 0;
    int expand_count = 0;

    std::vector<int> calculated_heights(children_.size(), 0);

    // PASS 1: Calculate Fixed and Percent sizes
    for (size_t i = 0; i < children_.size(); ++i) {
        const auto& policy = children_[i]->getHeightPolicy();
        
        if (policy.mode == SizeMode::Fixed) {
            calculated_heights[i] = policy.value;
            total_fixed_height += policy.value;
        } 
        else if (policy.mode == SizeMode::Percent) {
            int h = (height_ * policy.value) / 100;
            calculated_heights[i] = h;
            total_percent_height += h;
        } 
        else if (policy.mode == SizeMode::Expand) {
            expand_count++;
        }
    }

    // PASS 2: Calculate remaining space for 'Expand' widgets
    int remaining_space = std::max(0, height_ - total_fixed_height - total_percent_height);
    int expand_height = expand_count > 0 ? (remaining_space / expand_count) : 0;

    // PASS 3: Apply the geometry to the children
    int current_y = 0;
    for (size_t i = 0; i < children_.size(); ++i) {
        int final_height = calculated_heights[i];
        
        if (children_[i]->getHeightPolicy().mode == SizeMode::Expand) {
            final_height = expand_height;
        }

        // VBox forces children to stretch to its full width
        children_[i]->resizeAndMove(current_y, 0, final_height, width_);
        current_y += final_height;
    }
}

auto VBox::render() -> void {
    ncplane_erase(plane_);
    for (auto& child : children_) {
        child->render();
    }
}

auto VBox::handleInput(const ncinput& input) -> bool {
    // Bubble the input down to the children. First one to consume it wins.
    for (auto& child : children_) {
        if (child->handleInput(input)) {
            return true; 
        }
    }
    return false;
}

auto VBox::collectFocusable(std::vector<Widget*>& focusables) -> void {
    for (auto& child : children_) {
        child->collectFocusable(focusables);
    }
}

} // namespace notui