#include "notui/SplitContainer.h"
#include <algorithm>

namespace notui {

SplitContainer::SplitContainer(struct ncplane* parent, SplitDirection direction)
    : Widget(parent, 0, 0, 1, 1), direction_(direction) {}

auto SplitContainer::setFirst(std::unique_ptr<Widget> first) -> void {
    first_ = std::move(first);
    if (first_) {
        first_->setFocusManager(focus_manager_);
    }
    recalculateLayout();
}

auto SplitContainer::setSecond(std::unique_ptr<Widget> second) -> void {
    second_ = std::move(second);
    if (second_) {
        second_->setFocusManager(focus_manager_);
    }
    recalculateLayout();
}

auto SplitContainer::setFocusManager(FocusManager* manager) -> void {
    Widget::setFocusManager(manager);
    if (first_) {
        first_->setFocusManager(manager);
    }
    if (second_) {
        second_->setFocusManager(manager);
    }
}

auto SplitContainer::setSplitRatio(float ratio) -> void {
    split_ratio_ = std::clamp(ratio, 0.1f, 0.9f);
    recalculateLayout();
}

auto SplitContainer::resizeAndMove(int y, int x, int height, int width) -> void {
    Widget::resizeAndMove(y, x, height, width);
    recalculateLayout();
}

auto SplitContainer::recalculateLayout() -> void {
    if (direction_ == SplitDirection::Horizontal) {
        split_pos_ = static_cast<int>(width_ * split_ratio_);
        split_pos_ = std::clamp(split_pos_, 1, width_ - 2);

        if (first_) first_->resizeAndMove(0, 0, height_, split_pos_);
        if (second_) second_->resizeAndMove(0, split_pos_ + 1, height_, width_ - split_pos_ - 1);
    } else {
        split_pos_ = static_cast<int>(height_ * split_ratio_);
        split_pos_ = std::clamp(split_pos_, 1, height_ - 2);

        if (first_) first_->resizeAndMove(0, 0, split_pos_, width_);
        if (second_) second_->resizeAndMove(split_pos_ + 1, 0, height_ - split_pos_ - 1, width_);
    }
}

auto SplitContainer::render() -> void {
    ncplane_erase(plane_);

    // Draw the separator line
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, is_dragging_ ? 100 : 150, is_dragging_ ? 255 : 150, is_dragging_ ? 100 : 150);
    ncplane_set_base(plane_, " ", 0, channels);

    if (direction_ == SplitDirection::Horizontal) {
        for (int y = 0; y < height_; ++y) {
            ncplane_printf_yx(plane_, y, split_pos_, "│");
        }
    } else {
        for (int x = 0; x < width_; ++x) {
            ncplane_printf_yx(plane_, split_pos_, x, "─");
        }
    }

    if (first_) first_->render();
    if (second_) second_->render();
}

auto SplitContainer::handleInput(const ncinput& input) -> bool {
    int abs_y, abs_x;
    ncplane_yx(plane_, &abs_y, &abs_x);

    int rel_y = input.y - abs_y;
    int rel_x = input.x - abs_x;

    // --- DRAG LOGIC ---
    if (is_dragging_) {
        if (input.evtype == NCTYPE_RELEASE) {
            is_dragging_ = false;
            return true;
        }

        // Update split ratio based on mouse drag
        if (direction_ == SplitDirection::Horizontal) {
            setSplitRatio(static_cast<float>(rel_x) / width_);
        } else {
            setSplitRatio(static_cast<float>(rel_y) / height_);
        }
        return true;
    }

    // Check if they clicked the exact separator line
    bool over_separator = false;
    if (direction_ == SplitDirection::Horizontal && rel_x == split_pos_ && rel_y >= 0 && rel_y < height_) {
        over_separator = true;
    } else if (direction_ == SplitDirection::Vertical && rel_y == split_pos_ && rel_x >= 0 && rel_x < width_) {
        over_separator = true;
    }

    if (over_separator && input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON1) {
        is_dragging_ = true;
        return true;
    }

    // Bubble input to children
    if (first_ && first_->handleInput(input)) return true;
    if (second_ && second_->handleInput(input)) return true;

    return false;
}

auto SplitContainer::collectFocusable(std::vector<Widget*>& focusables) -> void {
    if (first_) {
        first_->collectFocusable(focusables);
    }
    if (second_) {
        second_->collectFocusable(focusables);
    }
}

} // namespace notui