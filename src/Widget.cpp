#include "notui/Widget.h"
#include <algorithm>

namespace notui {

Widget::~Widget() {
    destroy_planes();
}

void Widget::destroy_planes() {
    if (plane != nullptr) {
        ncplane_destroy(plane);
        plane = nullptr;
    }
}

void Widget::layout(struct ncplane* parent_plane, Point pos, Size size) {
    abs_y = pos.y;
    abs_x = pos.x;
    height = size.height;
    width = size.width;
    
    if (plane != nullptr) {
        ncplane_destroy(plane);
        plane = nullptr;
    }

    struct ncplane_options opts = {
        .y = pos.y, .x = pos.x,
        .rows = static_cast<unsigned>(size.height),
        .cols = static_cast<unsigned>(size.width),
        .userptr = this,
        .name = "widget",
        .resizecb = nullptr,
        .flags = 0
    };
    plane = ncplane_create(parent_plane, &opts);
}

void Widget::draw_box(const Style& box_style) const {
    if (!box_style.framed || width < 2 || height < 2) {
        return;
    }
    
    const char* corner_top_left = box_style.rounded_frame ? "╭" : "┌";
    const char* corner_top_right = box_style.rounded_frame ? "╮" : "┐";
    const char* corner_bottom_left = box_style.rounded_frame ? "╰" : "└";
    const char* corner_bottom_right = box_style.rounded_frame ? "╯" : "┘";
    const char* h_line = "─";
    const char* v_line = "│";

    uint64_t edge_channels = 0;
    uint64_t corner_channels = 0;
    ncchannels_set_fg_rgb8(&edge_channels, box_style.fg_r, box_style.fg_g, box_style.fg_b);
    if (box_style.transparent_bg) {
        ncchannels_set_bg_alpha(&edge_channels, NCALPHA_TRANSPARENT);
    } else {
        ncchannels_set_bg_rgb8(&edge_channels, box_style.bg_r, box_style.bg_g, box_style.bg_b);
    }

    corner_channels = edge_channels;

    ncplane_set_channels(plane, corner_channels);
    ncplane_putstr_yx(plane, 0, 0, corner_top_left);
    ncplane_putstr_yx(plane, 0, width - 1, corner_top_right);
    ncplane_putstr_yx(plane, height - 1, 0, corner_bottom_left);
    ncplane_putstr_yx(plane, height - 1, width - 1, corner_bottom_right);

    ncplane_set_channels(plane, edge_channels);
    for (int col = 1; col < width - 1; ++col) {
        ncplane_putstr_yx(plane, 0, col, h_line);
        ncplane_putstr_yx(plane, height - 1, col, h_line);
    }
    for (int row = 1; row < height - 1; ++row) {
        ncplane_putstr_yx(plane, row, 0, v_line);
        ncplane_putstr_yx(plane, row, width - 1, v_line);
    }

    if (!box_style.frame_title.empty() && width > static_cast<int>(box_style.frame_title.length()) + 4) {
        uint64_t title_channels = 0;
        ncchannels_set_fg_rgb8(&title_channels, box_style.fg_r, box_style.fg_g, box_style.fg_b);
        ncchannels_set_bg_rgb8(&title_channels, box_style.bg_r, box_style.bg_g, box_style.bg_b);
        ncplane_set_channels(plane, title_channels);
        ncplane_putstr_yx(plane, 0, 2, (" " + box_style.frame_title + " ").c_str());
    }
}

auto Widget::handle_input(const ncinput& nc_input) -> bool {
    emit("key_press", nc_input);
    return on_key_cb != nullptr && on_key_cb(this, nc_input);
}

void Widget::on_focus() {
    is_focused = true;
    emit("focus");
    if (on_focus_cb != nullptr) {
        on_focus_cb(this);
    }
}

void Widget::on_blur() {
    is_focused = false;
    emit("blur");
    if (on_blur_cb != nullptr) {
        on_blur_cb(this);
    }
}

auto Widget::contains_focus() -> bool {
    return is_focused;
}

auto Widget::get_widget_at(int pos_y, int pos_x) -> Widget* {
    if (plane == nullptr) {
        return nullptr;
    }
    int plane_y = ncplane_abs_y(plane);
    int plane_x = ncplane_abs_x(plane);
    if (pos_y >= plane_y && pos_y < plane_y + height && pos_x >= plane_x && pos_x < plane_x + width) {
        return this;
    }
    return nullptr;
}

void Widget::on(const std::string& event_name, std::function<void(const Event&)> callback) {
    if (callback) {
        event_listeners[event_name].push_back(std::move(callback));
    }
}

void Widget::emit(const std::string& event_name, const std::any& data) {
    Event event{ this, event_name, data };
    auto iter = event_listeners.find(event_name);
    if (iter != event_listeners.end()) {
        for (auto& listener : iter->second) {
            listener(event);
        }
    }
}

} // namespace notui