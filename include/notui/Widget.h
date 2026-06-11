#pragma once

#include <notcurses/notcurses.h>
#include "notui/Style.h"
#include "notui/Event.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <any>

namespace notui {

struct Widget {
    std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> event_listeners;

    Widget* parent = nullptr;
    int flex = 0; 
    int fixed_width = 0;
    int fixed_height = 0;
    
    int abs_y = 0, abs_x = 0, height = 0, width = 0;
    
    bool focusable = false;
    bool is_focused = false;
    bool is_overlay = false;
    struct ncplane* plane = nullptr;

    Style style;
    Style focused_style;

    std::function<void(Widget*)> on_focus_cb;
    std::function<void(Widget*)> on_blur_cb;
    std::function<bool(Widget*, const ncinput&)> on_key_cb;

    Widget() = default;
    virtual ~Widget();

    Widget(const Widget&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    Widget(Widget&&) = delete;
    auto operator=(Widget&&) -> Widget& = delete;

    void on(const std::string& event_name, std::function<void(const Event&)> callback);
    void emit(const std::string& event_name, const std::any& data = {});

    virtual void destroy_planes();
    virtual void layout(struct ncplane* parent_plane, Point pos, Size size);
    void draw_box(const Style& box_style) const;

    virtual void render() = 0;
    virtual auto handle_input(const ncinput& nc_input) -> bool;
    virtual void on_focus();
    virtual void on_blur();
    
    virtual auto contains_focus() -> bool;
    virtual auto get_widget_at(int pos_y, int pos_x) -> Widget*;
};

} // namespace notui