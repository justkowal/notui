#include "notui/Widget.h"
#include "notui/FocusManager.h"
#include "notui/Container.h"
#include <algorithm>

namespace notui {

auto StyleProxy::get_style() const -> Style& {
    if (!owner->extended_styles) {
        owner->extended_styles = std::make_unique<ExtendedStateStyles>();
    }
    return is_focused_style ? owner->extended_styles->focused_style : owner->extended_styles->disabled_style;
}

auto StyleProxy::get_style_const() const -> const Style& {
    if (!owner->extended_styles) {
        static const Style default_style;
        return default_style;
    }
    return is_focused_style ? owner->extended_styles->focused_style : owner->extended_styles->disabled_style;
}

auto StyleProxy::operator=(const Style& other) -> StyleProxy& {
    get_style() = other;
    return *this;
}

StyleProxy::operator Style&() {
    return get_style();
}

StyleProxy::operator const Style&() const {
    return get_style_const();
}

auto StyleProxy::operator&() -> Style* {
    return &get_style();
}

auto StyleProxy::operator&() const -> const Style* {
    return &get_style_const();
}

auto StyleProxy::operator->() -> Style* {
    return &get_style();
}

auto StyleProxy::operator->() const -> const Style* {
    return &get_style_const();
}

auto StyleProxy::bg(ColorRGB color) -> StyleProxy& {
    get_style().bg(color);
    return *this;
}

auto StyleProxy::fg(ColorRGB color) -> StyleProxy& {
    get_style().fg(color);
    return *this;
}

auto StyleProxy::attr(uint32_t attribute) -> StyleProxy& {
    get_style().attr(attribute);
    return *this;
}

auto StyleProxy::pad(Padding padding) -> StyleProxy& {
    get_style().pad(padding);
    return *this;
}

auto StyleProxy::transparent(bool enable_transparent) -> StyleProxy& {
    get_style().transparent(enable_transparent);
    return *this;
}

auto StyleProxy::frame(bool enable_frame, bool rounded, std::string title) -> StyleProxy& {
    get_style().frame(enable_frame, rounded, std::move(title));
    return *this;
}

void StyleProxy::apply(struct ncplane* plane) const {
    get_style_const().apply(plane);
}

Widget::Widget() : focused_style(this, true), disabled_style(this, false) {}

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

    if (parent_plane == nullptr || size.height <= 0 || size.width <= 0) {
        return;
    }

    struct ncplane_options opts = {};
    opts.y = pos.y;
    opts.x = pos.x;
    opts.rows = static_cast<unsigned>(size.height);
    opts.cols = static_cast<unsigned>(size.width);
    opts.userptr = this;
    opts.name = "widget";
    opts.resizecb = nullptr;
    opts.flags = 0;
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

void Widget::set_disabled(bool state) {
    if (disabled == state) {
        return;
    }
    disabled = state;
    if (disabled && is_focused && focus_manager != nullptr) {
        focus_manager->shift_focus_upstream_from(this);
    }
    if (focus_manager != nullptr) {
        if (disabled) {
            focus_manager->unregister_widget(this);
        } else {
            focus_manager->register_widget(this);
        }
    }
}

void Widget::set_parent(Widget* new_parent) {
    parent = new_parent;
    if (parent != nullptr) {
        set_focus_manager_recursive(parent->focus_manager);
    } else {
        set_focus_manager_recursive(nullptr);
    }
}

void Widget::set_focus_manager_recursive(FocusManager* manager) {
    if (focus_manager == manager) {
        return;
    }
    if (focus_manager != nullptr) {
        focus_manager->unregister_widget(this);
    }
    focus_manager = manager;
    if (focus_manager != nullptr) {
        focus_manager->register_widget(this);
    }
    if (auto* container = dynamic_cast<Container*>(this)) {
        for (const auto& child : container->get_children()) {
            if (child != nullptr) {
                child->set_focus_manager_recursive(manager);
            }
        }
    }
}

auto Widget::handle_input(const ncinput& nc_input) -> bool {
    if (disabled) {
        return false;
    }
    KeyPressEvent event_args{nc_input, false};
    emit("key", event_args);
    emit("key_press", nc_input);
    return event_args.handled;
}

void Widget::on_focus() {
    is_focused = true;
    emit("focus");
}

void Widget::on_blur() {
    is_focused = false;
    emit("blur");
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
        auto listeners_copy = iter->second;
        for (auto& listener : listeners_copy) {
            listener(event);
        }
    }

    if (event_name == "focus") {
        if (on_focus_cb) {
            on_focus_cb(this);
        }
    } else if (event_name == "blur") {
        if (on_blur_cb) {
            on_blur_cb(this);
        }
    } else if (event_name == "key") {
        if (on_key_cb) {
            if (auto* kp = std::any_cast<KeyPressEvent>(&data)) {
                if (on_key_cb(this, kp->input)) {
                    kp->handled = true;
                }
            } else if (auto* nc_in = std::any_cast<ncinput>(&data)) {
                on_key_cb(this, *nc_in);
            }
        }
    }
}

} 