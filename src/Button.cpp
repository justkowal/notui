#include "notui/Button.h"
#include <algorithm>
#include <utility>

namespace notui {

Button::Button(std::string label_text, std::function<void()> click_callback)
    : label(std::move(label_text)), on_click(std::move(click_callback)) {
    fixed_height = 1;
    fixed_width = static_cast<int>(label.length()) + 4;
    focusable = true;
    style = Theme::get_active().button_style;
    focused_style = Theme::get_active().button_focused;
    disabled_style = Theme::get_active().button_disabled;
}

void Button::render() {
    const Style* button_style_ptr = &style;
    if (disabled) {
        button_style_ptr = &disabled_style;
    } else if (is_focused) {
        button_style_ptr = &focused_style;
    }
    const Style& button_style = *button_style_ptr;

    uint64_t base_channels = 0;
    ncchannels_set_bg_alpha(&base_channels, NCALPHA_TRANSPARENT);
    ncchannels_set_fg_alpha(&base_channels, NCALPHA_TRANSPARENT);
    ncplane_set_base(plane, " ", 0, base_channels);
    ncplane_erase(plane);

    uint64_t channels = 0;
    ncchannels_set_bg_rgb8(&channels, button_style.bg_r, button_style.bg_g, button_style.bg_b);
    ncchannels_set_fg_rgb8(&channels, button_style.fg_r, button_style.fg_g, button_style.fg_b);
    ncplane_set_channels(plane, channels);
    ncplane_off_styles(plane, NCSTYLE_MASK);
    if (button_style.attrs != 0) {
        ncplane_set_styles(plane, button_style.attrs);
    }

    if (width >= 2) {
        for (int row = 0; row < height; ++row) {
            for (int col = 1; col < width - 1; ++col) {
                ncplane_putstr_yx(plane, row, col, " ");
            }
        }

        uint64_t left_channels = 0;
        ncchannels_set_fg_rgb8(&left_channels, button_style.bg_r, button_style.bg_g,
                               button_style.bg_b);
        ncchannels_set_bg_alpha(&left_channels, NCALPHA_TRANSPARENT);
        ncplane_set_channels(plane, left_channels);
        ncplane_putstr_yx(plane, height / 2, 0, "\uE0B6");

        uint64_t right_channels = 0;
        ncchannels_set_fg_rgb8(&right_channels, button_style.bg_r, button_style.bg_g,
                               button_style.bg_b);
        ncchannels_set_bg_alpha(&right_channels, NCALPHA_TRANSPARENT);
        ncplane_set_channels(plane, right_channels);
        ncplane_putstr_yx(plane, height / 2, width - 1, "\uE0B4");
    } else {
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                ncplane_putstr_yx(plane, row, col, " ");
            }
        }
    }

    ncplane_set_channels(plane, channels);
    if (width > 2) {
        int text_x = std::max(1, (width - static_cast<int>(label.length())) / 2);
        if (text_x + static_cast<int>(label.length()) > width - 1) {
            std::string truncated = label.substr(0, std::max(0, width - 2));
            ncplane_putstr_yx(plane, height / 2, 1, truncated.c_str());
        } else {
            ncplane_putstr_yx(plane, height / 2, text_x, label.c_str());
        }
    } else if (width > 0) {
        std::string truncated = label.substr(0, width);
        ncplane_putstr_yx(plane, height / 2, 0, truncated.c_str());
    }
}

auto Button::handle_input(const ncinput& nc_input) -> bool {
    if (disabled) {
        return false;
    }
    if ((nc_input.id == NCKEY_ENTER || nc_input.id == 13 || nc_input.id == 10 ||
         nc_input.id == NCKEY_BUTTON1) &&
        (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
        emit("click");
        if (on_click != nullptr) {
            on_click();
        }
        return true;
    }
    return false;
}

} 