#pragma once

#include <notcurses/notcurses.h>
#include <cstdint>
#include <string>
#include <utility>

namespace notui {

struct ColorRGB {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct Padding {
    int top = 0;
    int right = 0;
    int bottom = 0;
    int left = 0;
};

struct Point {
    int y = 0;
    int x = 0;
};

struct Size {
    int height = 0;
    int width = 0;
};

struct Style {
    uint8_t bg_r = 0, bg_g = 0, bg_b = 0;
    uint8_t fg_r = 255, fg_g = 255, fg_b = 255;
    uint32_t attrs = 0;
    int pt = 0, pr = 0, pb = 0, pl = 0; // padding: top, right, bottom, left
    bool transparent_bg = false;
    
    bool framed = false;
    bool rounded_frame = false;
    std::string frame_title;
    auto bg(ColorRGB color) -> Style& {
        bg_r = color.r;
        bg_g = color.g;
        bg_b = color.b;
        transparent_bg = false;
        return *this;
    }
    
    auto fg(ColorRGB color) -> Style& {
        fg_r = color.r;
        fg_g = color.g;
        fg_b = color.b;
        return *this;
    }
    
    auto attr(uint32_t attribute) -> Style& {
        attrs |= attribute;
        return *this;
    }
    
    auto pad(Padding padding) -> Style& {
        pt = padding.top;
        pr = padding.right;
        pb = padding.bottom;
        pl = padding.left;
        return *this;
    }
    
    auto transparent(bool enable_transparent) -> Style& {
        transparent_bg = enable_transparent;
        return *this;
    }
    
    auto frame(bool enable_frame, bool rounded = true, std::string title = "") -> Style& { 
        framed = enable_frame;
        rounded_frame = rounded;
        frame_title = std::move(title);
        return *this; 
    }

    void apply(struct ncplane* plane) const {
        uint64_t channels = 0;
        if (transparent_bg) {
            ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
        } else {
            ncchannels_set_bg_rgb8(&channels, bg_r, bg_g, bg_b);
        }
        ncchannels_set_fg_rgb8(&channels, fg_r, fg_g, fg_b);
        
        ncplane_set_base(plane, " ", 0, channels);
        ncplane_set_channels(plane, channels); 
        ncplane_off_styles(plane, NCSTYLE_MASK); 
        if (attrs != 0) {
            ncplane_set_styles(plane, attrs);
        }
    }
};

struct Theme {
    Style container_bg;
    Style button_style;
    Style button_focused;
    Style button_disabled;
    Style input_style;
    Style input_focused;
    Style input_disabled;
    Style label_style;

    static auto get_active() -> Theme& {
        static Theme active_theme = []() {
            Theme theme;
            theme.container_bg.bg({25, 25, 30}).fg({220, 220, 220});
            theme.button_style.bg({45, 50, 65}).fg({220, 220, 220}).pad({0, 2, 0, 2});
            theme.button_focused.bg({80, 150, 255}).fg({0, 0, 0}).attr(NCSTYLE_BOLD).pad({0, 2, 0, 2});
            theme.button_disabled.bg({35, 35, 35}).fg({95, 95, 95}).pad({0, 2, 0, 2});
            theme.input_style.bg({20, 20, 20}).fg({150, 150, 150}).pad({0, 1, 0, 1});
            theme.input_focused.bg({35, 35, 35}).fg({255, 255, 100}).pad({0, 1, 0, 1}).attr(NCSTYLE_BOLD);
            theme.input_disabled.bg({20, 20, 20}).fg({75, 75, 75}).pad({0, 1, 0, 1});
            theme.label_style.transparent(true).fg({200, 200, 200}).pad({0, 1, 0, 1});
            return theme;
        }();
        return active_theme;
    }
};

} // namespace notui