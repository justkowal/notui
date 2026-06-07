#include "notui/Label.h"
#include <algorithm>

namespace notui {

Label::Label(struct ncplane* parent, std::string text, TextAlignment align)
    : Widget(parent, 0, 0, 1, 1), text_(std::move(text)), alignment_(align) {
    
    // Labels typically take exactly 1 row of vertical space, but expand to fill horizontal space
    setHeightPolicy(SizeMode::Fixed, 1);
    setWidthPolicy(SizeMode::Expand);

    // Labels look best with a transparent/default background in most themes, 
    // but we will default to standard style colors for safety.
}

auto Label::setText(std::string text) -> void {
    text_ = std::move(text);
    // The next render loop will automatically pick up the new text
}

auto Label::setAlignment(TextAlignment align) -> void {
    alignment_ = align;
}

auto Label::render() -> void {
    ncplane_erase(plane_);

    // Apply styles
    uint64_t channels = style_.getNormalChannels();
    ncplane_set_base(plane_, " ", 0, channels);
    ncplane_set_fg_rgb8(plane_, style_.fg.r, style_.fg.g, style_.fg.b);
    ncplane_set_bg_rgb8(plane_, style_.bg.r, style_.bg.g, style_.bg.b);

    // Calculate alignment offsets
    int text_length = static_cast<int>(text_.length());
    int print_x = 0;

    switch (alignment_) {
        case TextAlignment::Left:   
            print_x = 0; 
            break;
        case TextAlignment::Center: 
            print_x = std::max(0, (width_ - text_length) / 2); 
            break;
        case TextAlignment::Right:  
            print_x = std::max(0, width_ - text_length); 
            break;
    }

    // Draw the text exactly on row 0, shifted by the calculated X offset
    ncplane_printf_yx(plane_, 0, print_x, "%s", text_.c_str());
}

auto Label::handleInput(const ncinput& /*input*/) -> bool {
    // Labels are purely visual; they never consume mouse clicks or keyboard input
    return false;
}

} // namespace notui