#include "notui/ScrollArea.h"
#include <algorithm>
#include <ranges>

namespace notui {

// ── anonymous-namespace helpers (no header declarations needed) ───────────────
namespace {

// Bundles the commonly-computed layout/render dimensions so that helpers
// never receive a long list of adjacent same-type ints.
struct ScrollDims {
    int border_offset   = 0;
    int usable_h        = 0;
    int usable_w        = 0;
    int total_content_h = 0;
    int max_scroll      = 0;
    int outer_height    = 0;  // widget height (passed to helpers so they need no bare int)
    int outer_width     = 0;  // widget width
    int scroll_y        = 0;  // current scroll position
};

// Bundles the three scalar inputs to compute_dims so no adjacent ints appear.
struct WidgetSize {
    Size size;
    int scroll_y = 0;
};

auto compute_dims(const Style& style,
                  const std::vector<std::shared_ptr<Widget>>& children,
                  const WidgetSize& widget_size) -> ScrollDims {
    ScrollDims dims;
    dims.outer_height  = widget_size.size.height;
    dims.outer_width   = widget_size.size.width;
    dims.scroll_y      = widget_size.scroll_y;
    dims.border_offset = style.framed ? 1 : 0;
    dims.usable_h      = std::max(0, widget_size.size.height - style.pt - style.pb - (style.framed ? 2 : 0));
    dims.usable_w      = std::max(0, widget_size.size.width  - style.pl - style.pr - (style.framed ? 2 : 0));
    for (const auto& child : children) {
        if (child != nullptr) {
            dims.total_content_h += child->fixed_height;
        }
    }
    dims.max_scroll = std::max(0, dims.total_content_h - dims.usable_h);
    return dims;
}

void draw_border(struct ncplane* overlay, const Style& style, const ScrollDims& dims) {
    int height = dims.outer_height;
    int width  = dims.outer_width;
    if (!style.framed || width < 2 || height < 2) {
        return;
    }

    const char* top_left  = style.rounded_frame ? "╭" : "┌";
    const char* top_right = style.rounded_frame ? "╮" : "┐";
    const char* bot_left  = style.rounded_frame ? "╰" : "└";
    const char* bot_right = style.rounded_frame ? "╯" : "┘";
    const char* h_line = "─";
    const char* v_line = "│";

    uint64_t edge_ch = 0;
    ncchannels_set_fg_rgb8(&edge_ch, style.fg_r, style.fg_g, style.fg_b);
    if (style.transparent_bg) {
        ncchannels_set_bg_alpha(&edge_ch, NCALPHA_TRANSPARENT);
    } else {
        ncchannels_set_bg_rgb8(&edge_ch, style.bg_r, style.bg_g, style.bg_b);
    }

    ncplane_set_channels(overlay, edge_ch);
    ncplane_putstr_yx(overlay, 0, 0, top_left);
    ncplane_putstr_yx(overlay, 0, width - 1, top_right);
    ncplane_putstr_yx(overlay, height - 1, 0, bot_left);
    ncplane_putstr_yx(overlay, height - 1, width - 1, bot_right);

    for (int col = 1; col < width - 1; ++col) {
        ncplane_putstr_yx(overlay, 0, col, h_line);
        ncplane_putstr_yx(overlay, height - 1, col, h_line);
    }
    for (int row = 1; row < height - 1; ++row) {
        ncplane_putstr_yx(overlay, row, 0, v_line);
        ncplane_putstr_yx(overlay, row, width - 1, v_line);
    }

    if (!style.frame_title.empty() && width > static_cast<int>(style.frame_title.length()) + 4) {
        ncplane_putstr_yx(overlay, 0, 2, (" " + style.frame_title + " ").c_str());
    }
}

void draw_scrollbar(struct ncplane* overlay, const Style& style,
                    const ScrollDims& dims) {
    if (dims.total_content_h <= dims.usable_h) {
        return;
    }

    int track_x      = dims.outer_width - style.pr - 1 - dims.border_offset;
    int track_start  = style.pt + dims.border_offset;

    float thumb_ratio = static_cast<float>(dims.usable_h) / static_cast<float>(dims.total_content_h);
    int   thumb_h     = std::max(1, static_cast<int>(static_cast<float>(dims.usable_h) * thumb_ratio));

    int thumb_y = track_start;
    if (dims.max_scroll > 0) {
        float frac = static_cast<float>(dims.scroll_y) / static_cast<float>(dims.max_scroll);
        thumb_y += static_cast<int>(frac * static_cast<float>(dims.usable_h - thumb_h));
    }

    uint64_t track_ch = 0;
    uint64_t thumb_ch = 0;
    ncchannels_set_fg_rgb8(&track_ch, 80, 80, 80);
    ncchannels_set_bg_rgb8(&track_ch, style.bg_r, style.bg_g, style.bg_b);
    ncchannels_set_fg_rgb8(&thumb_ch, 100, 255, 100);
    ncchannels_set_bg_rgb8(&thumb_ch, style.bg_r, style.bg_g, style.bg_b);

    for (int idx = 0; idx < dims.usable_h; ++idx) {
        int abs_row = track_start + idx;
        if (abs_row >= thumb_y && abs_row < thumb_y + thumb_h) {
            ncplane_set_channels(overlay, thumb_ch);
            ncplane_putstr_yx(overlay, abs_row, track_x, "█");
        } else {
            ncplane_set_channels(overlay, track_ch);
            ncplane_putstr_yx(overlay, abs_row, track_x, "│");
        }
    }
}

void draw_fade_overlays(struct ncplane* overlay, const Style& style,
                        const ScrollDims& dims) {
    if (dims.usable_h <= 4) {
        return;
    }

    int content_w = dims.usable_w - (dims.total_content_h > dims.usable_h ? 1 : 0);
    int top_y     = style.pt + dims.border_offset;
    int bottom_y  = dims.outer_height - style.pb - dims.border_offset - 1;
    int inner_x   = style.pl + dims.border_offset;

    // Opaque mask — conceals border/padding zones
    uint64_t opaque_ch = 0;
    ncchannels_set_fg_rgb8(&opaque_ch, style.bg_r, style.bg_g, style.bg_b);
    ncchannels_set_fg_alpha(&opaque_ch, NCALPHA_OPAQUE);
    ncchannels_set_bg_rgb8(&opaque_ch, style.bg_r, style.bg_g, style.bg_b);
    ncchannels_set_bg_alpha(&opaque_ch, NCALPHA_OPAQUE);

    // Strong blend cell (innermost fade row)
    nccell blend_strong = {};
    nccell_set_fg_alpha(&blend_strong, NCALPHA_BLEND);
    nccell_set_bg_alpha(&blend_strong, NCALPHA_BLEND);
    ncchannels_set_fg_rgb8(&blend_strong.channels, style.bg_r, style.bg_g, style.bg_b);
    ncchannels_set_bg_rgb8(&blend_strong.channels, style.bg_r, style.bg_g, style.bg_b);

    // Soft blend cell (outermost fade row — slightly brighter bg)
    nccell blend_soft = {};
    nccell_set_fg_alpha(&blend_soft, NCALPHA_BLEND);
    nccell_set_bg_alpha(&blend_soft, NCALPHA_BLEND);
    uint8_t soft_r = static_cast<uint8_t>(std::min(255, style.bg_r + 20));
    uint8_t soft_g = static_cast<uint8_t>(std::min(255, style.bg_g + 20));
    uint8_t soft_b = static_cast<uint8_t>(std::min(255, style.bg_b + 20));
    ncchannels_set_fg_rgb8(&blend_soft.channels, soft_r, soft_g, soft_b);
    ncchannels_set_bg_rgb8(&blend_soft.channels, soft_r, soft_g, soft_b);

    // Fill padding/border zones with opaque mask
    ncplane_set_channels(overlay, opaque_ch);
    for (int row = dims.border_offset; row < top_y; ++row) {
        for (int col = 0; col < content_w; ++col) {
            ncplane_putstr_yx(overlay, row, inner_x + col, " ");
        }
    }
    for (int row = bottom_y + 1; row < dims.outer_height - dims.border_offset; ++row) {
        for (int col = 0; col < content_w; ++col) {
            ncplane_putstr_yx(overlay, row, inner_x + col, " ");
        }
    }

    // Top fade gradient — only when scrolled down
    if (dims.scroll_y > 0) {
        if (top_y + 1 < bottom_y) {
            for (int col = 0; col < content_w; ++col) {
                ncplane_putc_yx(overlay, top_y + 1, inner_x + col, &blend_soft);
            }
        }
        for (int col = 0; col < content_w; ++col) {
            ncplane_putc_yx(overlay, top_y, inner_x + col, &blend_strong);
        }
    }

    // Bottom fade gradient — only when more content is below
    if (dims.scroll_y < dims.max_scroll) {
        if (bottom_y - 1 > top_y) {
            for (int col = 0; col < content_w; ++col) {
                ncplane_putc_yx(overlay, bottom_y - 1, inner_x + col, &blend_soft);
            }
        }
        for (int col = 0; col < content_w; ++col) {
            ncplane_putc_yx(overlay, bottom_y, inner_x + col, &blend_strong);
        }
    }
}

void clamp_scroll_for_focus(std::vector<std::shared_ptr<Widget>>& children,
                             int& scroll_y, int usable_h) {
    int vis_y = 0;
    for (auto& child : children) {
        if (child == nullptr) {
            continue;
        }
        if (child->contains_focus()) {
            if (vis_y < scroll_y) {
                scroll_y = vis_y;
            } else if (vis_y + child->fixed_height > scroll_y + usable_h) {
                scroll_y = vis_y + child->fixed_height - usable_h;
            }
        }
        vis_y += child->fixed_height;
    }
}

} // anonymous namespace

// ── ScrollArea implementation ─────────────────────────────────────────────────

ScrollArea::ScrollArea() {
    flex = 1;
    focusable = true;
}

void ScrollArea::destroy_planes() {
    if (overlay_plane != nullptr) {
        ncplane_destroy(overlay_plane);
        overlay_plane = nullptr;
    }
    Container::destroy_planes();
}

void ScrollArea::layout(struct ncplane* parent_plane, Point pos, Size size) {
    Widget::layout(parent_plane, pos, size);

    if (overlay_plane != nullptr) {
        ncplane_destroy(overlay_plane);
        overlay_plane = nullptr;
    }

    struct ncplane_options opts = {
        .y = 0, .x = 0,
        .rows = static_cast<unsigned>(size.height),
        .cols = static_cast<unsigned>(size.width),
        .userptr = this,
        .name = "scroll_overlay",
        .resizecb = nullptr,
        .flags = 0
    };
    overlay_plane = ncplane_create(plane, &opts);

    const ScrollDims dims = compute_dims(style, children, WidgetSize{size, scroll_y});
    int child_w = dims.usable_w - 1; // Leave room for scrollbar

    clamp_scroll_for_focus(children, scroll_y, dims.usable_h);

    if (scroll_y > dims.max_scroll) {
        scroll_y = dims.max_scroll;
    }
    if (scroll_y < 0) {
        scroll_y = 0;
    }

    // Place each child within the visible scroll viewport
    int vis_y = 0;
    for (auto& child : children) {
        if (child == nullptr) {
            continue;
        }
        int y_in_plane = style.pt + dims.border_offset + (vis_y - scroll_y);

        // Only layout children whose planes lie entirely within [0, size.height).
        if (y_in_plane >= 0 && y_in_plane + child->fixed_height <= size.height) {
            child->layout(plane, Point{y_in_plane, style.pl + dims.border_offset},
                          Size{child->fixed_height, child_w});
        } else {
            child->destroy_planes();
        }
        vis_y += child->fixed_height;
    }
}

void ScrollArea::render() {
    // 1. Render all children on the base plane first
    Container::render();

    if (overlay_plane == nullptr) {
        return;
    }

    // 2. Clear overlay, make it transparent in the middle
    uint64_t base_channels = 0;
    ncchannels_set_bg_alpha(&base_channels, NCALPHA_TRANSPARENT);
    ncchannels_set_fg_alpha(&base_channels, NCALPHA_TRANSPARENT);
    ncplane_set_base(overlay_plane, "", 0, base_channels);
    ncplane_erase(overlay_plane);

    const ScrollDims dims = compute_dims(style, children, WidgetSize{Size{height, width}, scroll_y});

    // 3. Border on overlay (masks card overflows)
    draw_border(overlay_plane, style, dims);

    // 4. Scrollbar on overlay
    draw_scrollbar(overlay_plane, style, dims);

    // 5. Top/bottom fade-out gradient overlays
    draw_fade_overlays(overlay_plane, style, dims);

    // 6. Raise overlay to top so it masks the child cards
    ncplane_move_top(overlay_plane);
}

auto ScrollArea::handle_input(const ncinput& nc_input) -> bool {
    const ScrollDims dims = compute_dims(style, children, WidgetSize{Size{height, width}, scroll_y});

    if (dims.max_scroll == 0) {
        return false;
    }

    bool changed = false;

    // Arrow key / wheel scrolling
    if (nc_input.id == NCKEY_SCROLL_UP || nc_input.id == NCKEY_UP) {
        if (scroll_y > 0) {
            scroll_y -= 1;
            changed = true;
        }
    } else if (nc_input.id == NCKEY_SCROLL_DOWN || nc_input.id == NCKEY_DOWN) {
        if (scroll_y < dims.max_scroll) {
            scroll_y += 1;
            changed = true;
        }
    }

    // Scrollbar track click / drag
    int track_x     = ncplane_abs_x(plane) + width - style.pr - 1 - dims.border_offset;
    int track_start = ncplane_abs_y(plane) + style.pt + dims.border_offset;
    bool is_click   = (nc_input.id == NCKEY_BUTTON1 || nc_input.id == NCKEY_MOTION);
    bool on_track   = (nc_input.x == track_x &&
                       nc_input.y >= track_start &&
                       nc_input.y < track_start + dims.usable_h);

    if (is_click && on_track) {
        float thumb_ratio = static_cast<float>(dims.usable_h) / static_cast<float>(dims.total_content_h);
        int   thumb_h     = std::max(1, static_cast<int>(static_cast<float>(dims.usable_h) * thumb_ratio));
        int   travel      = dims.usable_h - thumb_h;
        int   adjusted    = (nc_input.y - track_start) - (thumb_h / 2);

        if (adjusted < 0) {
            adjusted = 0;
        }
        if (adjusted > travel) {
            adjusted = travel;
        }

        if (travel > 0) {
            float frac = static_cast<float>(adjusted) / static_cast<float>(travel);
            scroll_y   = static_cast<int>(frac * static_cast<float>(dims.max_scroll));
        }
        changed = true;
    }

    if (changed) {
        if (scroll_y < 0) {
            scroll_y = 0;
        }
        if (scroll_y > dims.max_scroll) {
            scroll_y = dims.max_scroll;
        }
        struct ncplane* parent_plane = ncplane_parent(plane);
        this->layout(parent_plane, Point{abs_y, abs_x}, Size{height, width});
        return true;
    }
    return false;
}

auto ScrollArea::get_widget_at(int pos_y, int pos_x) -> Widget* {
    if (plane == nullptr) {
        return nullptr;
    }
    int plane_y = ncplane_abs_y(plane);
    int plane_x = ncplane_abs_x(plane);
    if (pos_y >= plane_y && pos_y < plane_y + height &&
        pos_x >= plane_x && pos_x < plane_x + width) {
        int border_offset = style.framed ? 1 : 0;
        int top_y    = plane_y + style.pt + border_offset;
        int bottom_y = plane_y + height - style.pb - border_offset - 1;

        // Only route events to children within the visible viewport
        if (pos_y >= top_y && pos_y <= bottom_y) {
            for (auto const& child : children | std::views::reverse) {
                if (child != nullptr) {
                    Widget* hit = child->get_widget_at(pos_y, pos_x);
                    if (hit != nullptr) {
                        return hit;
                    }
                }
            }
        }
        return this;
    }
    return nullptr;
}

} // namespace notui
