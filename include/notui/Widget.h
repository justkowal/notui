#pragma once

#include <notcurses/notcurses.h>
#include "notui/Style.h"
#include "notui/Event.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <any>
#include <memory>

namespace notui {

class FocusManager;
struct Widget;

struct KeyPressEvent {
    ncinput input{};
    mutable bool handled = false;
};

struct ExtendedStateStyles {
    Style focused_style;
    Style disabled_style;
};

struct StyleProxy {
private:
    Widget* owner = nullptr;
    bool is_focused_style = false;

    [[nodiscard]] auto get_style() const -> Style&;
    [[nodiscard]] auto get_style_const() const -> const Style&;

public:
    StyleProxy() = default;
    StyleProxy(Widget* owner_in, bool is_focused_style_in)
        : owner(owner_in), is_focused_style(is_focused_style_in) {}

    auto operator=(const Style& other) -> StyleProxy&;

    operator Style&();
    operator const Style&() const;

    auto operator&() -> Style*;
    auto operator&() const -> const Style*;

    auto operator->() -> Style*;
    auto operator->() const -> const Style*;

    auto bg(ColorRGB color) -> StyleProxy&;
    auto fg(ColorRGB color) -> StyleProxy&;
    auto attr(uint32_t attribute) -> StyleProxy&;
    auto pad(Padding padding) -> StyleProxy&;
    auto transparent(bool enable_transparent) -> StyleProxy&;
    auto frame(bool enable_frame, bool rounded = true, std::string title = "") -> StyleProxy&;

    void apply(struct ncplane* plane) const;
};

struct IOverlay {
    virtual ~IOverlay() = default;
    IOverlay() = default;
    IOverlay(const IOverlay&) = default;
    auto operator=(const IOverlay&) -> IOverlay& = default;
    IOverlay(IOverlay&&) = default;
    auto operator=(IOverlay&&) -> IOverlay& = default;

    virtual void raise_to_top() = 0;
    virtual auto is_active_overlay() -> bool = 0;
};

struct Widget {
    std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> event_listeners;

    Widget* parent = nullptr;
    int flex = 0; 
    int fixed_width = 0;
    int fixed_height = 0;
    
    int abs_y = 0, abs_x = 0, height = 0, width = 0;
    
    bool focusable = false;
    bool is_focused = false;
    
    // Deprecated: Use IOverlay interface instead
    bool is_overlay = false;
    
    bool disabled = false;
    struct ncplane* plane = nullptr;

    Style style;
    std::unique_ptr<ExtendedStateStyles> extended_styles;
    StyleProxy focused_style;
    StyleProxy disabled_style;

    FocusManager* focus_manager = nullptr;

    std::function<void(Widget*)> on_focus_cb;
    std::function<void(Widget*)> on_blur_cb;
    std::function<bool(Widget*, const ncinput&)> on_key_cb;

    Widget();
    virtual ~Widget();

    Widget(const Widget&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    Widget(Widget&&) = delete;
    auto operator=(Widget&&) -> Widget& = delete;

    void on(const std::string& event_name, std::function<void(const Event&)> callback);
    void emit(const std::string& event_name, const std::any& data = {});

    virtual void set_disabled(bool state);
    void set_parent(Widget* new_parent);
    void set_focus_manager_recursive(FocusManager* manager);

    virtual void destroy_planes();
    virtual void layout(struct ncplane* parent_plane, Point pos, Size size);
    void draw_box(const Style& box_style) const;

    virtual void render() = 0;
    virtual auto handle_input(const ncinput& nc_input) -> bool;
    virtual void on_focus();
    virtual void on_blur();
    
    virtual auto contains_focus() -> bool;
    virtual auto get_widget_at(int pos_y, int pos_x) -> Widget*;
    
    // Deprecated: Use IOverlay interface instead
    virtual void raise_to_top() {
        if (plane != nullptr) {
            ncplane_move_top(plane);
        }
    }

    // Deprecated: Use IOverlay interface instead
    virtual auto is_active_overlay() -> bool {
        return is_overlay;
    }
};

} // namespace notui
