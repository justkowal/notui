#include "notui/Frame.h"
#include <algorithm>

namespace notui {

Frame::Frame(struct ncplane* parent, int y, int x, int height, int width, std::string title)
    : Widget(parent, y, x, height, width), title_(std::move(title)) {}

auto Frame::setChild(std::unique_ptr<Widget> child) -> void {
    child_ = std::move(child);
    
    // Immediately calculate the padding geometry for the new child
    if (child_) {
        child_->setFocusManager(focus_manager_);
        // Child's x and y are relative to the Frame's plane.
        // We offset by 1 on all sides for the border.
        int child_height = std::max(1, height_ - 2);
        int child_width  = std::max(1, width_ - 2);
        child_->resizeAndMove(1, 1, child_height, child_width);
    }
}

auto Frame::resizeAndMove(int y, int x, int height, int width) -> void {
    Widget::resizeAndMove(y, x, height, width);

    // If the frame itself is resized (e.g. by a VBox), we must resize the child to match
    if (child_) {
        int child_height = std::max(1, height_ - 2);
        int child_width  = std::max(1, width_ - 2);
        child_->resizeAndMove(1, 1, child_height, child_width);
    }
}

auto Frame::render() -> void {
    ncplane_erase(plane_);

    // Set standard terminal colors (White text, default background)
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, 220, 220, 220); 
    ncplane_set_base(plane_, " ", 0, channels);

    // 1. Prepare Notcurses line-drawing cells
    nccell ul = NCCELL_TRIVIAL_INITIALIZER, ur = NCCELL_TRIVIAL_INITIALIZER;
    nccell ll = NCCELL_TRIVIAL_INITIALIZER, lr = NCCELL_TRIVIAL_INITIALIZER;
    nccell hl = NCCELL_TRIVIAL_INITIALIZER, vl = NCCELL_TRIVIAL_INITIALIZER;

    // Load the rounded Unicode border characters
    nccell_load(plane_, &ul, "╭"); nccell_load(plane_, &ur, "╮");
    nccell_load(plane_, &ll, "╰"); nccell_load(plane_, &lr, "╯");
    nccell_load(plane_, &hl, "─"); nccell_load(plane_, &vl, "│");

    // 2. Draw the perimeter
    ncplane_perimeter(plane_, &ul, &ur, &ll, &lr, &hl, &vl, 0);

    // 3. Draw the title if one exists, perfectly placed on the top border
    if (!title_.empty()) {
        ncplane_printf_yx(plane_, 0, 2, " %s ", title_.c_str());
    }

    // 4. Render the inner content
    if (child_) {
        child_->render();
    }
}

auto Frame::handleInput(const ncinput& input) -> bool {
    // Frames are purely structural. Bubble the input down to the child.
    if (child_) {
        return child_->handleInput(input);
    }
    return false;
}

auto Frame::setFocusManager(FocusManager* manager) -> void {
    Widget::setFocusManager(manager);
    if (child_) {
        child_->setFocusManager(manager);
    }
}

auto Frame::collectFocusable(std::vector<Widget*>& focusables) -> void {
    if (child_) {
        child_->collectFocusable(focusables);
    }
}

} // namespace notui