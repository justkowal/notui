#pragma once

#include "notui/Widget.h"
#include "notui/Style.h"
#include <string>
#include <functional>

namespace notui {

class Button : public Widget {
public:
    // We default to size 1x1 because VBox/HBox will immediately resize it anyway
    Button(struct ncplane* parent, std::string label, std::function<void()> on_click);

    auto acceptsFocus() const -> bool override { return true; }
    auto setStyle(const Style& style) -> void { style_ = style; }
    [[nodiscard]] auto getStyle() const -> const Style& { return style_; }

    auto render() -> void override;
    auto handleInput(const ncinput& input) -> bool override;

private:
    std::string label_;
    std::function<void()> on_click_;
    Style style_;
    
    bool is_hovered_ = false;
};

} // namespace notui