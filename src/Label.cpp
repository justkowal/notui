#include "notui/Label.h"
#include <algorithm>
#include <sstream>

namespace notui {

namespace {

auto wrap_text(const std::string& input, int max_w) -> std::vector<std::string> {
    std::vector<std::string> lines;
    if (max_w <= 0) {
        lines.emplace_back(input);
        return lines;
    }

    std::stringstream stream(input);
    std::string line;
    while (std::getline(stream, line, '\n')) {
        std::stringstream word_stream(line);
        std::string word;
        std::string current_line;
        while (word_stream >> word) {
            if (current_line.empty()) {
                current_line = word;
            } else if (static_cast<int>(current_line.length() + 1 + word.length()) <= max_w) {
                current_line += " " + word;
            } else {
                lines.emplace_back(current_line);
                current_line = word;
            }
        }
        if (!current_line.empty()) {
            lines.emplace_back(current_line);
        } else if (line.empty()) {
            lines.emplace_back("");
        }
    }
    if (lines.empty()) {
        lines.emplace_back("");
    }
    return lines;
}

} 

Label::Label(std::string label_text, Size fixed_size, bool center) 
    : text(std::move(label_text)), centered(center) { 
    fixed_height = fixed_size.height; 
    fixed_width = fixed_size.width; 
    style = Theme::get_active().label_style;
}

void Label::set_text(const std::string& new_text) { 
    text = new_text; 
}

void Label::set_highlight_query(std::string query) {
    highlight_query = std::move(query);
}

auto Label::get_highlight_query() const -> const std::string& {
    return highlight_query;
}

void Label::render() {
    if (plane == nullptr) {
        return;
    }

    style.apply(plane);
    ncplane_erase(plane);
    draw_box(style);

    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, width - style.pl - style.pr - (style.framed ? 2 : 0));

    int max_text_width = std::max(1, usable_w);
    std::vector<std::string> wrapped = wrap_text(text, max_text_width);

    int start_y = style.pt + border_offset + std::max(0, (usable_h - static_cast<int>(wrapped.size())) / 2);

    for (size_t i = 0; i < wrapped.size(); ++i) {
        int line_y = start_y + static_cast<int>(i);
        if (line_y >= height - style.pb - border_offset) {
            break;
        }
        int text_x = centered ? std::max(style.pl + border_offset, (width - static_cast<int>(wrapped[i].length())) / 2) : style.pl + border_offset;
        
        std::string line_str = wrapped[i];
        if (highlight_query.empty()) {
            ncplane_putstr_yx(plane, line_y, text_x, line_str.c_str());
        } else {
            std::string line_lower = line_str;
            std::transform(line_lower.begin(), line_lower.end(), line_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::string query_lower = highlight_query;
            std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            size_t last_pos = 0;
            size_t pos = line_lower.find(query_lower, 0);

            ncplane_cursor_move_yx(plane, line_y, text_x);

            while (pos != std::string::npos) {
                if (pos > last_pos) {
                    std::string prefix = line_str.substr(last_pos, pos - last_pos);
                    ncplane_putstr(plane, prefix.c_str());
                }

                ncplane_set_fg_rgb8(plane, 255, 255, 0);
                ncplane_on_styles(plane, NCSTYLE_BOLD);

                std::string match = line_str.substr(pos, query_lower.length());
                ncplane_putstr(plane, match.c_str());

                ncplane_off_styles(plane, NCSTYLE_MASK);
                if (style.attrs != 0) {
                    ncplane_set_styles(plane, style.attrs);
                }
                ncplane_set_fg_rgb8(plane, style.fg_r, style.fg_g, style.fg_b);

                last_pos = pos + query_lower.length();
                pos = line_lower.find(query_lower, last_pos);
            }

            if (last_pos < line_str.length()) {
                std::string suffix = line_str.substr(last_pos);
                ncplane_putstr(plane, suffix.c_str());
            }
        }
    }
}

} 