#pragma once

#include <notcurses/notcurses.h>
#include <cstdint>

namespace notui {

struct Color {
    uint32_t r = 255;
    uint32_t g = 255;
    uint32_t b = 255;
};

struct Style {
    Color fg         = {220, 220, 220}; // Off-white text
    Color bg         = {50, 50, 60};    // Dark gray background
    Color focus_fg   = {255, 255, 255}; // Pure white text on hover
    Color focus_bg   = {80, 80, 100};   // Lighter gray/blue on hover

    // Helpers to instantly get the Notcurses 64-bit color channels
    [[nodiscard]] auto getNormalChannels() const -> uint64_t {
        uint64_t channels = 0;
        ncchannels_set_fg_rgb8(&channels, fg.r, fg.g, fg.b);
        ncchannels_set_bg_rgb8(&channels, bg.r, bg.g, bg.b);
        return channels;
    }

    [[nodiscard]] auto getFocusChannels() const -> uint64_t {
        uint64_t channels = 0;
        ncchannels_set_fg_rgb8(&channels, focus_fg.r, focus_fg.g, focus_fg.b);
        ncchannels_set_bg_rgb8(&channels, focus_bg.r, focus_bg.g, focus_bg.b);
        return channels;
    }
};

} // namespace notui