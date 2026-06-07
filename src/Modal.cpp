#include <notui/Modal.h>

namespace notui {

Modal::Modal(struct ncplane* parent, int y, int x, int height, int width, std::string title)
    : Widget(parent, y, x, height, width), title_(std::move(title)) {
    bringToFront();
}

auto Modal::setContent(std::unique_ptr<Widget> content) -> void {
    content_ = std::move(content);
    // Automatically size the content to fit inside the modal's 1-character border
    if (content_) {
        content_->setFocusManager(focus_manager_);
        content_->resizeAndMove(1, 1, height_ - 2, width_ - 2);
    }
}

auto Modal::setFocusManager(FocusManager* manager) -> void {
    Widget::setFocusManager(manager);
    if (content_) {
        content_->setFocusManager(manager);
    }
}

auto Modal::bringToFront() -> void {
    ncplane_move_top(plane_);
}

auto Modal::render() -> void {
    ncplane_erase(plane_);

    // 1. Draw an opaque background to hide elements behind the modal
    uint64_t channels = 0;
    ncchannels_set_fg_rgb8(&channels, 230, 230, 230); // White text
    ncchannels_set_bg_rgb8(&channels, 30, 30, 40);    // Dark slate background
    
    if (is_dragging_) {
        ncchannels_set_fg_rgb8(&channels, 100, 255, 100); // Turn border green when dragging
    }
    
    ncplane_set_base(plane_, " ", 0, channels);

    // 2. Draw standard Notcurses borders
    nccell ul = NCCELL_TRIVIAL_INITIALIZER, ur = NCCELL_TRIVIAL_INITIALIZER;
    nccell ll = NCCELL_TRIVIAL_INITIALIZER, lr = NCCELL_TRIVIAL_INITIALIZER;
    nccell hl = NCCELL_TRIVIAL_INITIALIZER, vl = NCCELL_TRIVIAL_INITIALIZER;

    nccell_load(plane_, &ul, "╭"); nccell_load(plane_, &ur, "╮");
    nccell_load(plane_, &ll, "╰"); nccell_load(plane_, &lr, "╯");
    nccell_load(plane_, &hl, "─"); nccell_load(plane_, &vl, "│");

    ncplane_perimeter(plane_, &ul, &ur, &ll, &lr, &hl, &vl, 0);

    // 3. Draw Title Bar
    if (!title_.empty()) {
        ncplane_printf_yx(plane_, 0, 2, " %s ", title_.c_str());
    }

    // 4. Render injected content
    if (content_) {
        content_->render();
    }
}

auto Modal::handleInput(const ncinput& input) -> bool {
    // Determine the absolute position of this modal in the terminal
    int abs_y, abs_x;
    ncplane_yx(plane_, &abs_y, &abs_x);

    // --- DRAG LOGIC ---
    if (is_dragging_) {
        // Stop dragging on mouse release
        if (input.evtype == NCTYPE_RELEASE) {
            is_dragging_ = false;
            return true;
        }

        // Calculate delta and move the plane
        int delta_y = input.y - drag_start_y_;
        int delta_x = input.x - drag_start_x_;
        ncplane_move_yx(plane_, plane_start_y_ + delta_y, plane_start_x_ + delta_x);
        return true; // Consume event so the UI behind it doesn't react
    }

    // Check if the user clicked inside our modal bounding box
    bool hit_modal = (input.y >= abs_y && input.y < abs_y + height_ && 
                      input.x >= abs_x && input.x < abs_x + width_);

    if (hit_modal) {
        // Did they left-click exactly on the top row (Title Bar)?
        if (input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON1 && input.y == abs_y) {
            is_dragging_ = true;
            drag_start_y_ = input.y;
            drag_start_x_ = input.x;
            plane_start_y_ = abs_y;
            plane_start_x_ = abs_x;
            return true;
        }

        // Pass input down to the inner content
        if (content_ && content_->handleInput(input)) {
            return true;
        }

        // Even if the inner content didn't use the click/key, we return true 
        // to trap the input and prevent it from bleeding through to the dashboard below.
        return true; 
    }

    return false; // User clicked/typed completely outside the modal
}

auto Modal::collectFocusable(std::vector<Widget*>& focusables) -> void {
    if (content_) {
        content_->collectFocusable(focusables);
    }
}

} // namespace notui