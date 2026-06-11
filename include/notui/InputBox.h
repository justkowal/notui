#pragma once

#include "notui/Widget.h"
#include <string>
#include <functional>
#include <sstream>
#include <algorithm>
#include <type_traits>
#include <array>

namespace notui {

template <typename ValueType>
struct InputBox : public Widget {
private:
    std::string buffer;
    std::string placeholder;
    std::function<bool(const std::string&)> validator;
    int cursor_pos = 0; 
    int scroll_offset = 0; 
    std::function<void(const std::string&)> on_change;

public:
    explicit InputBox(std::string placeholder_text, int width_val, std::function<bool(const std::string&)> validator_fn = nullptr) 
        : placeholder(std::move(placeholder_text)), validator(std::move(validator_fn)) {
        fixed_height = 1; // NOLINT(cppcoreguidelines-prefer-member-initializer)
        fixed_width = width_val; // NOLINT(cppcoreguidelines-prefer-member-initializer)
        focusable = true; // NOLINT(cppcoreguidelines-prefer-member-initializer)
        style = Theme::get_active().input_style; // NOLINT(cppcoreguidelines-prefer-member-initializer)
        focused_style = Theme::get_active().input_focused; // NOLINT(cppcoreguidelines-prefer-member-initializer)
    }
    ~InputBox() override = default;

    InputBox(const InputBox&) = delete;
    auto operator=(const InputBox&) -> InputBox& = delete;
    InputBox(InputBox&&) = delete;
    auto operator=(InputBox&&) -> InputBox& = delete;

    [[nodiscard]] auto get_value() -> ValueType {
        if constexpr (std::is_same_v<ValueType, std::string>) {
            return buffer;
        }
        ValueType value = ValueType();
        if (!buffer.empty()) {
            std::stringstream stream(buffer);
            stream >> value;
        }
        return value;
    }

    void set_value(const std::string& val) {
        buffer = val;
        cursor_pos = static_cast<int>(buffer.length());
        scroll_offset = 0;
    }

    void render() override {
        if (plane == nullptr) {
            return;
        }

        const Style& input_style = is_focused ? focused_style : style;
        input_style.apply(plane);
        ncplane_erase(plane);
        draw_box(input_style);

        int display_width = width - input_style.pl - input_style.pr;
        if (display_width < 1) {
            display_width = 1;
        }

        if (cursor_pos < scroll_offset) {
            scroll_offset = cursor_pos;
        } else if (cursor_pos >= scroll_offset + display_width) {
            scroll_offset = cursor_pos - display_width + 1;
        }

        int max_scroll = std::max(0, static_cast<int>(buffer.length()) - display_width + 1);
        if (scroll_offset > max_scroll) {
            scroll_offset = max_scroll;
        }

        std::string display = buffer.empty() && !is_focused ? placeholder : buffer;
        std::string visible = display.length() > static_cast<size_t>(scroll_offset)
            ? display.substr(static_cast<size_t>(scroll_offset), static_cast<size_t>(display_width))
            : "";

        ncplane_putstr_yx(plane, height / 2, input_style.pl, visible.c_str());

        if (is_focused) {
            ncplane_set_bg_rgb8(plane, 200, 200, 200);
            ncplane_set_fg_rgb8(plane, 0, 0, 0);       
            ncplane_off_styles(plane, NCSTYLE_MASK); 
            
            int cursor_x = input_style.pl + cursor_pos - scroll_offset;
            if (cursor_pos < static_cast<int>(buffer.length())) {
                std::array<char, 2> c_str = {buffer[static_cast<size_t>(cursor_pos)], '\0'};
                ncplane_putstr_yx(plane, height / 2, cursor_x, c_str.data());
            } else {
                ncplane_putstr_yx(plane, height / 2, cursor_x, " ");
            }
        }
    }

    auto handle_input(const ncinput& nc_input) -> bool override { // NOLINT(readability-function-cognitive-complexity)
        if (nc_input.evtype == NCTYPE_RELEASE) {
            return false;
        }
        
        if (nc_input.id == NCKEY_LEFT) {
            if (cursor_pos > 0) { 
                cursor_pos--; 
                return true; 
            }
            return false; 
        }
        if (nc_input.id == NCKEY_RIGHT) {
            if (cursor_pos < static_cast<int>(buffer.length())) { 
                cursor_pos++; 
                return true; 
            }
            return false; 
        }
        if (nc_input.id == NCKEY_UP || nc_input.id == NCKEY_DOWN || nc_input.id == NCKEY_SCROLL_UP || nc_input.id == NCKEY_SCROLL_DOWN) {
            return false; // bubble vertical events to scrollarea
        }

        std::string prospective_buffer = buffer;
        bool changed = false;

        if (nc_input.id == NCKEY_BACKSPACE || nc_input.id == 8 || nc_input.id == 127) {
            if (cursor_pos > 0) { 
                prospective_buffer.erase(static_cast<size_t>(cursor_pos - 1), 1); 
                changed = true; 
            }
        } else if (nc_input.id == NCKEY_DEL) {
            if (cursor_pos < static_cast<int>(buffer.length())) { 
                prospective_buffer.erase(static_cast<size_t>(cursor_pos), 1); 
                changed = true; 
            }
        } else if (nc_input.id >= 32 && nc_input.id <= 126) {
            prospective_buffer.insert(static_cast<size_t>(cursor_pos), 1, static_cast<char>(nc_input.id));
            changed = true;
        }

        if (changed) {
            if (!validator || validator(prospective_buffer)) {
                buffer = prospective_buffer;
                if (nc_input.id == NCKEY_BACKSPACE || nc_input.id == 8 || nc_input.id == 127) {
                    cursor_pos--;
                } else if (nc_input.id >= 32 && nc_input.id <= 126) {
                    cursor_pos++;
                }
                emit("change", get_value());
                if (on_change != nullptr) {
                    on_change(buffer);
                }
            }
            return true;
        }
        return false; 
    }
};

} // namespace notui