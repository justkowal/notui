#include "notui/SplitBox.h"
#include <algorithm>
#include <notcurses/notcurses.h>

namespace notui {

SplitBox::SplitBox(Orientation orientation, double default_split_percent)
    : orientation_(orientation), split_percent_(default_split_percent) {
    focusable = true;
}

void SplitBox::layout(struct ncplane* parent_plane, Point pos, Size size) {
    Widget::layout(parent_plane, pos, size);

    if (children.empty()) {
        return;
    }

    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, size.height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, size.width - style.pl - style.pr - (style.framed ? 2 : 0));

    if (!split_enabled_ || children.size() < 2) {
        // Lay out first child with full usable area
        if (children[0] != nullptr) {
            children[0]->layout(plane, Point{style.pt + border_offset, style.pl + border_offset}, Size{usable_h, usable_w});
        }
        // Destroy planes for any other children so they are not drawn/active
        for (size_t i = 1; i < children.size(); ++i) {
            if (children[i] != nullptr) {
                children[i]->destroy_planes();
            }
        }
        return;
    }

    if (orientation_ == Orientation::Horizontal) {
        int w_total = usable_w - 1; // 1 column for divider
        if (w_total < 0) w_total = 0;

        int w1 = static_cast<int>(w_total * split_percent_);
        int min_w = 3;
        if (w_total >= 2 * min_w) {
            w1 = std::clamp(w1, min_w, w_total - min_w);
        } else {
            w1 = w_total / 2;
        }
        int w2 = w_total - w1;

        if (children[0] != nullptr) {
            children[0]->layout(plane, Point{style.pt + border_offset, style.pl + border_offset}, Size{usable_h, w1});
        }
        if (children[1] != nullptr) {
            children[1]->layout(plane, Point{style.pt + border_offset, style.pl + border_offset + w1 + 1}, Size{usable_h, w2});
        }
    } else {
        int h_total = usable_h - 1; // 1 row for divider
        if (h_total < 0) h_total = 0;

        int h1 = static_cast<int>(h_total * split_percent_);
        int min_h = 2;
        if (h_total >= 2 * min_h) {
            h1 = std::clamp(h1, min_h, h_total - min_h);
        } else {
            h1 = h_total / 2;
        }
        int h2 = h_total - h1;

        if (children[0] != nullptr) {
            children[0]->layout(plane, Point{style.pt + border_offset, style.pl + border_offset}, Size{h1, usable_w});
        }
        if (children[1] != nullptr) {
            children[1]->layout(plane, Point{style.pt + border_offset + h1 + 1, style.pl + border_offset}, Size{h2, usable_w});
        }
    }
}

void SplitBox::render() {
    Container::render();

    if (!split_enabled_ || children.size() < 2 || plane == nullptr) {
        return;
    }

    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, width - style.pl - style.pr - (style.framed ? 2 : 0));

    uint64_t divider_channels = 0;
    ncchannels_set_fg_rgb8(&divider_channels, 100, 100, 120);
    if (style.transparent_bg) {
        ncchannels_set_bg_alpha(&divider_channels, NCALPHA_TRANSPARENT);
    } else {
        ncchannels_set_bg_rgb8(&divider_channels, style.bg_r, style.bg_g, style.bg_b);
    }
    ncplane_set_channels(plane, divider_channels);

    if (orientation_ == Orientation::Horizontal) {
        int w_total = usable_w - 1;
        if (w_total < 0) w_total = 0;
        int w1 = static_cast<int>(w_total * split_percent_);
        int min_w = 3;
        if (w_total >= 2 * min_w) {
            w1 = std::clamp(w1, min_w, w_total - min_w);
        } else {
            w1 = w_total / 2;
        }

        int divider_x = style.pl + border_offset + w1;
        for (int y = 0; y < usable_h; ++y) {
            ncplane_putstr_yx(plane, style.pt + border_offset + y, divider_x, "│");
        }
    } else {
        int h_total = usable_h - 1;
        if (h_total < 0) h_total = 0;
        int h1 = static_cast<int>(h_total * split_percent_);
        int min_h = 2;
        if (h_total >= 2 * min_h) {
            h1 = std::clamp(h1, min_h, h_total - min_h);
        } else {
            h1 = h_total / 2;
        }

        int divider_y = style.pt + border_offset + h1;
        for (int x = 0; x < usable_w; ++x) {
            ncplane_putstr_yx(plane, divider_y, style.pl + border_offset + x, "─");
        }
    }
}

auto SplitBox::handle_input(const ncinput& nc_input) -> bool {
    if (!split_enabled_ || children.size() < 2 || plane == nullptr) {
        return false;
    }

    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, width - style.pl - style.pr - (style.framed ? 2 : 0));

    // Handle button release
    if (nc_input.evtype == NCTYPE_RELEASE) {
        if (is_dragging_) {
            is_dragging_ = false;
            return true;
        }
        return false;
    }

    bool is_mouse = (nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON1) || 
                     nc_input.id == static_cast<uint32_t>(NCKEY_MOTION));

    if (!is_mouse) {
        return false;
    }

    int abs_y_coord = ncplane_abs_y(plane);
    int abs_x_coord = ncplane_abs_x(plane);

    if (orientation_ == Orientation::Horizontal) {
        int w_total = usable_w - 1;
        if (w_total <= 0) return false;

        int w1 = static_cast<int>(w_total * split_percent_);
        int min_w = 3;
        if (w_total >= 2 * min_w) {
            w1 = std::clamp(w1, min_w, w_total - min_w);
        } else {
            w1 = w_total / 2;
        }

        int divider_abs_x = abs_x_coord + style.pl + border_offset + w1;

        if (!is_dragging_) {
            bool on_divider = (nc_input.x == divider_abs_x &&
                               nc_input.y >= abs_y_coord + style.pt + border_offset &&
                               nc_input.y < abs_y_coord + style.pt + border_offset + usable_h);

            if (on_divider && nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON1) &&
                (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
                is_dragging_ = true;
                return true;
            }
        } else {
            int rel_mouse_x = nc_input.x - abs_x_coord - style.pl - border_offset;
            int new_w1 = rel_mouse_x;
            if (w_total >= 2 * min_w) {
                new_w1 = std::clamp(new_w1, min_w, w_total - min_w);
            } else {
                new_w1 = w_total / 2;
            }
            double new_percent = static_cast<double>(new_w1) / w_total;
            if (new_percent != split_percent_) {
                split_percent_ = new_percent;
                struct ncplane* parent_plane = ncplane_parent(plane);
                this->layout(parent_plane, Point{this->abs_y, this->abs_x}, Size{this->height, this->width});
            }
            return true;
        }
    } else {
        int h_total = usable_h - 1;
        if (h_total <= 0) return false;

        int h1 = static_cast<int>(h_total * split_percent_);
        int min_h = 2;
        if (h_total >= 2 * min_h) {
            h1 = std::clamp(h1, min_h, h_total - min_h);
        } else {
            h1 = h_total / 2;
        }

        int divider_abs_y = abs_y_coord + style.pt + border_offset + h1;

        if (!is_dragging_) {
            bool on_divider = (nc_input.y == divider_abs_y &&
                               nc_input.x >= abs_x_coord + style.pl + border_offset &&
                               nc_input.x < abs_x_coord + style.pl + border_offset + usable_w);

            if (on_divider && nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON1) &&
                (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
                is_dragging_ = true;
                return true;
            }
        } else {
            int rel_mouse_y = nc_input.y - abs_y_coord - style.pt - border_offset;
            int new_h1 = rel_mouse_y;
            if (h_total >= 2 * min_h) {
                new_h1 = std::clamp(new_h1, min_h, h_total - min_h);
            } else {
                new_h1 = h_total / 2;
            }
            double new_percent = static_cast<double>(new_h1) / h_total;
            if (new_percent != split_percent_) {
                split_percent_ = new_percent;
                struct ncplane* parent_plane = ncplane_parent(plane);
                this->layout(parent_plane, Point{this->abs_y, this->abs_x}, Size{this->height, this->width});
            }
            return true;
        }
    }

    return false;
}

auto SplitBox::get_widget_at(int pos_y, int pos_x) -> Widget* {
    if (is_dragging_) {
        return this;
    }
    return Container::get_widget_at(pos_y, pos_x);
}

void SplitBox::set_split_enabled(bool enabled) {
    if (split_enabled_ != enabled) {
        split_enabled_ = enabled;
        if (plane != nullptr) {
            struct ncplane* parent_plane = ncplane_parent(plane);
            this->layout(parent_plane, Point{this->abs_y, this->abs_x}, Size{this->height, this->width});
        }
    }
}

auto SplitBox::is_split_enabled() const -> bool {
    return split_enabled_;
}

void SplitBox::set_split_percent(double percent) {
    split_percent_ = std::clamp(percent, 0.0, 1.0);
    if (plane != nullptr) {
        struct ncplane* parent_plane = ncplane_parent(plane);
        this->layout(parent_plane, Point{this->abs_y, this->abs_x}, Size{this->height, this->width});
    }
}

auto SplitBox::get_split_percent() const -> double {
    return split_percent_;
}

} // namespace notui
