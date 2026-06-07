#include <notui/Widget.h>
#include <notui/FocusManager.h>

namespace notui {

Widget::Widget(struct ncplane* parent, int pos_y, int pos_x, int height, int width)
    : height_(height), width_(width) {
    
    struct ncplane_options opts = {};
    opts.y = pos_y;
    opts.x = pos_x;
    opts.rows = height;
    opts.cols = width;
    
    // Create the dedicated plane for this widget
    plane_ = ncplane_create(parent, &opts);
}

Widget::~Widget() {
    if (plane_ != nullptr) {
        ncplane_destroy(plane_);
        plane_ = nullptr; // Crucial: prevent double-free
    }
}

auto Widget::resizeAndMove(int y_pos, int x_pos, int new_height, int new_width) -> void {
    height_ = new_height;
    width_ = new_width;
    ncplane_resize_simple(plane_, new_height, new_width);
    ncplane_move_yx(plane_, y_pos, x_pos);
}

auto Widget::collectFocusable(std::vector<Widget*>& widgets) -> void {
    if (acceptsFocus()) {
        widgets.push_back(this);
    }
}

auto Widget::setFocusManager(FocusManager* manager) -> void {
    focus_manager_ = manager;
}

auto Widget::requestFocus() -> void {
    if (focus_manager_ != nullptr) {
        focus_manager_->focusWidget(this);
    }
}

auto Widget::getAbsolutePosition(int& y, int& x) const -> void {
    if (plane_ == nullptr) {
        y = 0;
        x = 0;
        return;
    }

    ncplane_yx(plane_, &y, &x);
}

} // namespace notui