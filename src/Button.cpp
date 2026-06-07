#include "notui/Button.h"
#include <algorithm>

namespace notui {

Button::Button(struct ncplane* parent, std::string label, std::function<void()> on_click)
    : Widget(parent, 0, 0, 1, 1), 
      label_(std::move(label)), 
      on_click_(std::move(on_click)) {
    
    // Buttons typically have a fixed height of 1 row, but expand horizontally
    setHeightPolicy(SizeMode::Fixed, 1);
    setWidthPolicy(SizeMode::Expand);
}

auto Button::render() -> void {
    ncplane_erase(plane_);

    const bool highlighted = is_focused_ || is_hovered_;

    // 1. Apply the correct color based on hover or focus state
    uint64_t channels = highlighted ? style_.getFocusChannels() : style_.getNormalChannels();
    ncplane_set_base(plane_, " ", 0, channels);

    // Apply colors explicitly for the text rendering
    if (highlighted) {
        ncplane_set_fg_rgb8(plane_, style_.focus_fg.r, style_.focus_fg.g, style_.focus_fg.b);
        ncplane_set_bg_rgb8(plane_, style_.focus_bg.r, style_.focus_bg.g, style_.focus_bg.b);
    } else {
        ncplane_set_fg_rgb8(plane_, style_.fg.r, style_.fg.g, style_.fg.b);
        ncplane_set_bg_rgb8(plane_, style_.bg.r, style_.bg.g, style_.bg.b);
    }

    // 2. Center the text dynamically
    int text_length = static_cast<int>(label_.length());
    int text_x = std::max(0, (width_ - text_length) / 2);
    int text_y = std::max(0, height_ / 2);

    ncplane_printf_yx(plane_, text_y, text_x, "%s", label_.c_str());
}

auto Button::handleInput(const ncinput& input) -> bool {
    // Determine the absolute coordinates of this button on the screen
    int abs_y, abs_x;
    ncplane_yx(plane_, &abs_y, &abs_x);

    // Check if the mouse cursor is currently over this button
    bool hit = (input.y >= abs_y && input.y < abs_y + height_ && 
                input.x >= abs_x && input.x < abs_x + width_);

    // If it's a mouse movement event, update the hover state
    if (input.evtype == NCTYPE_RELEASE || input.evtype == NCTYPE_PRESS || input.id == NCKEY_MOTION) {
        if (hit != is_hovered_) {
            is_hovered_ = hit;
            // The next render loop will automatically draw the new color
        }
    }

    // If they actually clicked the left mouse button while hovering
    if (hit && input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON1) {
        requestFocus();
        if (on_click_) {
            on_click_(); // Trigger the callback
        }
        return true; // We consumed this input
    }

    if (is_focused_ && (input.evtype == NCTYPE_PRESS || input.evtype == NCTYPE_REPEAT)) {
        if (input.id == NCKEY_ENTER || input.id == NCKEY_SPACE) {
            if (on_click_) {
                on_click_();
            }
            return true;
        }
    }

    return false;
}

} // namespace notui