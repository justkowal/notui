#pragma once

#include "notui/Widget.h"
#include <string>
#include <vector>

namespace notui {

class TextArea : public Widget {
private:
    std::vector<std::string> buffer_lines;
    std::string placeholder;
    int cursor_y   = 0;
    int cursor_x   = 0;
    int scroll_top = 0;

public:
    explicit TextArea(std::string placeholder_hint = "", Size fixed_size = {});
    ~TextArea() override = default;

    TextArea(const TextArea&) = delete;
    auto operator=(const TextArea&) -> TextArea& = delete;
    TextArea(TextArea&&) = delete;
    auto operator=(TextArea&&) -> TextArea& = delete;

    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;

    [[nodiscard]] auto get_text() const -> std::string;
    void set_text(const std::string& text);
};

} // namespace notui
