#pragma once

#include "notui/Widget.h"
#include "notui/Style.h"
#include <string>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <cctype>

namespace notui {

// Defaults to std::string if no type is provided
template <typename T = std::string>
class InputBox : public Widget {
public:
    // The callback now returns an std::optional<T>. 
    // If they type invalid junk, you get std::nullopt, making it perfectly exception-safe.
    InputBox(struct ncplane* parent, std::string placeholder = "", std::function<void(std::optional<T>)> on_submit = nullptr)
        : Widget(parent, 0, 0, 1, 1), placeholder_(std::move(placeholder)), on_submit_(std::move(on_submit)) {
        setHeightPolicy(SizeMode::Fixed, 1);
        setWidthPolicy(SizeMode::Expand);
    }

    auto acceptsFocus() const -> bool override { return true; }

    auto clear() -> void { 
        text_.clear(); 
        cursor_pos_ = 0;
        scroll_offset_ = 0;
    }

    auto setStyle(const Style& style) -> void { style_ = style; }

    auto render() -> void override {
        ncplane_erase(plane_);

        uint64_t channels = 0;
        if (is_focused_) {
            ncchannels_set_fg_rgb8(&channels, style_.focus_fg.r, style_.focus_fg.g, style_.focus_fg.b);
            ncchannels_set_bg_rgb8(&channels, style_.focus_bg.r, style_.focus_bg.g, style_.focus_bg.b);
        } else {
            ncchannels_set_fg_rgb8(&channels, style_.fg.r, style_.fg.g, style_.fg.b);
            ncchannels_set_bg_rgb8(&channels, style_.bg.r, style_.bg.g, style_.bg.b);
        }
        ncplane_set_base(plane_, " ", 0, channels);

        std::string display_text = text_.empty() && !is_focused_ ? placeholder_ : text_;
        if (text_.empty() && !is_focused_) {
            ncplane_set_fg_rgb8(plane_, 150, 150, 150); // Dim placeholder text
        }

        std::string visible_text = display_text.substr(scroll_offset_, width_);
        ncplane_printf_yx(plane_, 0, 0, "%s", visible_text.c_str());

        if (is_focused_) {
            int visual_cursor_x = cursor_pos_ - scroll_offset_;
            if (visual_cursor_x >= 0 && visual_cursor_x < width_) {
                char cursor_char = (cursor_pos_ < text_.length()) ? text_[cursor_pos_] : ' ';
                ncplane_set_fg_rgb8(plane_, style_.focus_bg.r, style_.focus_bg.g, style_.focus_bg.b);
                ncplane_set_bg_rgb8(plane_, style_.focus_fg.r, style_.focus_fg.g, style_.focus_fg.b);
                ncplane_printf_yx(plane_, 0, visual_cursor_x, "%c", cursor_char);
            }
        }
    }

    auto handleInput(const ncinput& input) -> bool override {
        int abs_y, abs_x;
        ncplane_yx(plane_, &abs_y, &abs_x);

        bool clicked_inside = (input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON1 &&
                               input.y >= abs_y && input.y < abs_y + height_ && 
                               input.x >= abs_x && input.x < abs_x + width_);

        if (input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON1) {
            if (!clicked_inside) return false;
            requestFocus();
            return true; 
        }

        if (!is_focused_) return false;

        if (input.evtype == NCTYPE_PRESS || input.evtype == NCTYPE_REPEAT) {
            
            if (input.id == NCKEY_ENTER) {
                if (on_submit_) on_submit_(parseValue());
                if (focus_manager_ != nullptr) {
                    focus_manager_->clearFocus();
                }
                return true;
            } 
            else if (input.id == NCKEY_BACKSPACE && cursor_pos_ > 0) {
                text_.erase(cursor_pos_ - 1, 1);
                cursor_pos_--;
            } 
            else if (input.id == NCKEY_LEFT && cursor_pos_ > 0) {
                cursor_pos_--;
            }
            else if (input.id == NCKEY_LEFT && cursor_pos_ == 0) {
                return false;
            } 
            else if (input.id == NCKEY_RIGHT && cursor_pos_ < text_.length()) {
                cursor_pos_++;
            }
            else if (input.id == NCKEY_RIGHT && cursor_pos_ >= text_.length()) {
                return false;
            }
            else if (ncinput_valid_p(&input) && input.id >= 32 && input.id <= 126) {
                char c = static_cast<char>(input.id);

                // --- TEMPLATE METAPROGRAMMING: INPUT FILTERING ---
                // If T is a number (int, float, double), physically prevent typing letters
                if constexpr (std::is_arithmetic_v<T>) {
                    bool is_valid_numeric = std::isdigit(c) || c == '-' || c == '.';
                    if (!is_valid_numeric) {
                        return true; // Consume the keystroke, but do nothing
                    }
                }

                text_.insert(cursor_pos_, 1, c);
                cursor_pos_++;
            }

            if (cursor_pos_ < scroll_offset_) {
                scroll_offset_ = cursor_pos_;
            } else if (cursor_pos_ >= scroll_offset_ + width_) {
                scroll_offset_ = cursor_pos_ - width_ + 1;
            }
            return true;
        }
        return is_focused_;
    }

private:
    std::string text_;
    std::string placeholder_;
    std::function<void(std::optional<T>)> on_submit_;
    Style style_;
    int cursor_pos_ = 0;   
    int scroll_offset_ = 0;

    // --- TEMPLATE METAPROGRAMMING: TYPE PARSING ---
    auto parseValue() const -> std::optional<T> {
        if (text_.empty()) return std::nullopt;

        try {
            if constexpr (std::is_same_v<T, std::string>) {
                return text_;
            } 
            else if constexpr (std::is_same_v<T, int>) {
                return std::stoi(text_);
            } 
            else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
                return std::stod(text_);
            }
        } catch (const std::exception&) {
            // User typed "-" or "12.3.4" which passed the keystroke filter but isn't a valid number
            return std::nullopt; 
        }
        return std::nullopt;
    }
};

} // namespace notui