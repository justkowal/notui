#pragma once

#include "notui/Widget.h"
#include "notui/Style.h"
#include <string>

namespace notui {

enum class TextAlignment { Left, Center, Right };

class Label : public Widget {
public:
    Label(struct ncplane* parent, std::string text, TextAlignment align = TextAlignment::Left);

    // Dynamic updaters
    auto setText(std::string text) -> void;
    auto setAlignment(TextAlignment align) -> void;
    
    // Theming
    auto setStyle(const Style& style) -> void { style_ = style; }
    [[nodiscard]] auto getStyle() const -> const Style& { return style_; }

    auto render() -> void override;
    auto handleInput(const ncinput& input) -> bool override;

private:
    std::string text_;
    TextAlignment alignment_;
    Style style_;
};

} // namespace notui