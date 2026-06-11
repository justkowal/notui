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

} // namespace

Label::Label(std::string label_text, Size fixed_size, bool center) 
    : text(std::move(label_text)), centered(center) { 
    fixed_height = fixed_size.height; 
    fixed_width = fixed_size.width; 
    style = Theme::get_active().label_style;
}

void Label::set_text(const std::string& new_text) { 
    text = new_text; 
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
        ncplane_putstr_yx(plane, line_y, text_x, wrapped[i].c_str());
    }
}

} // namespace notui