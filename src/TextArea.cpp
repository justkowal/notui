#include "notui/TextArea.h"
#include <algorithm>
#include <sstream>
#include <array>

namespace notui {
namespace {

struct CursorCtx {
    int client_y = 0;
    int client_x = 0;
    int usable_w = 0;
    int row      = 0;
    int cursor_x = 0;
};

void draw_cursor(struct ncplane* plane, const Style& cur_style,
                 const CursorCtx& ctx, const std::string& line) {
    int visual_x = ctx.client_x + ctx.cursor_x;
    if (visual_x >= ctx.client_x + ctx.usable_w) {
        return;
    }

    ncplane_set_bg_rgb8(plane, 200, 200, 200);
    ncplane_set_fg_rgb8(plane, 0, 0, 0);
    ncplane_off_styles(plane, NCSTYLE_MASK);

    if (ctx.cursor_x < static_cast<int>(line.length())) {
        std::array<char, 2> c_str = {line[ctx.cursor_x], '\0'};
        ncplane_putstr_yx(plane, ctx.client_y + ctx.row, visual_x, c_str.data());
    } else {
        ncplane_putstr_yx(plane, ctx.client_y + ctx.row, visual_x, " ");
    }

    uint64_t channels = 0;
    ncchannels_set_bg_rgb8(&channels, cur_style.bg_r, cur_style.bg_g, cur_style.bg_b);
    ncchannels_set_fg_rgb8(&channels, cur_style.fg_r, cur_style.fg_g, cur_style.fg_b);
    ncplane_set_channels(plane, channels);
}

auto apply_motion_key(const ncinput& nc_input,
                      int& cursor_x, int& cursor_y,
                      const std::vector<std::string>& buffer_lines) -> bool {
    if (nc_input.id == NCKEY_LEFT) {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = static_cast<int>(buffer_lines[cursor_y].length());
        }
        return true;
    }
    if (nc_input.id == NCKEY_RIGHT) {
        if (cursor_x < static_cast<int>(buffer_lines[cursor_y].length())) {
            cursor_x++;
        } else if (cursor_y < static_cast<int>(buffer_lines.size()) - 1) {
            cursor_y++;
            cursor_x = 0;
        }
        return true;
    }
    if (nc_input.id == NCKEY_UP && cursor_y > 0) {
        cursor_y--;
        cursor_x = std::min(cursor_x, static_cast<int>(buffer_lines[cursor_y].length()));
        return true;
    }
    if (nc_input.id == NCKEY_DOWN && cursor_y < static_cast<int>(buffer_lines.size()) - 1) {
        cursor_y++;
        cursor_x = std::min(cursor_x, static_cast<int>(buffer_lines[cursor_y].length()));
        return true;
    }
    return false;
}

auto apply_edit_key(const ncinput& nc_input,
                    int& cursor_x, int& cursor_y,
                    std::vector<std::string>& buffer_lines) -> bool {
    bool is_enter     = (nc_input.id == NCKEY_ENTER || nc_input.id == 13 || nc_input.id == 10);
    bool is_backspace = (nc_input.id == NCKEY_BACKSPACE || nc_input.id == 8 || nc_input.id == 127);
    bool is_delete    = (nc_input.id == NCKEY_DEL);
    bool is_printable = (nc_input.id >= 32 && nc_input.id <= 126);

    if (is_enter) {
        std::string& cur_line = buffer_lines[cursor_y];
        std::string next_line = cur_line.substr(cursor_x);
        cur_line = cur_line.substr(0, cursor_x);
        buffer_lines.insert(buffer_lines.begin() + cursor_y + 1, next_line);
        cursor_y++;
        cursor_x = 0;
        return true;
    }

    if (is_backspace) {
        if (cursor_x > 0) {
            buffer_lines[cursor_y].erase(cursor_x - 1, 1);
            cursor_x--;
            return true;
        }
        if (cursor_y > 0) {
            cursor_x = static_cast<int>(buffer_lines[cursor_y - 1].length());
            buffer_lines[cursor_y - 1] += buffer_lines[cursor_y];
            buffer_lines.erase(buffer_lines.begin() + cursor_y);
            cursor_y--;
            return true;
        }
        return false;
    }

    if (is_delete) {
        if (cursor_x < static_cast<int>(buffer_lines[cursor_y].length())) {
            buffer_lines[cursor_y].erase(cursor_x, 1);
            return true;
        }
        if (cursor_y < static_cast<int>(buffer_lines.size()) - 1) {
            buffer_lines[cursor_y] += buffer_lines[cursor_y + 1];
            buffer_lines.erase(buffer_lines.begin() + cursor_y + 1);
            return true;
        }
        return false;
    }

    if (is_printable) {
        buffer_lines[cursor_y].insert(cursor_x, 1, static_cast<char>(nc_input.id));
        cursor_x++;
        return true;
    }

    return false;
}

} 

TextArea::TextArea(std::string placeholder_hint, Size fixed_size)
    : placeholder(std::move(placeholder_hint)) {
    fixed_height  = fixed_size.height != 0 ? fixed_size.height : 5;
    fixed_width   = fixed_size.width != 0 ? fixed_size.width : 30;
    focusable     = true;
    style         = Theme::get_active().input_style;
    focused_style = Theme::get_active().input_focused;
    disabled_style = Theme::get_active().input_disabled;
    buffer_lines.emplace_back("");
}

auto TextArea::get_text() const -> std::string {
    std::string result;
    for (size_t idx = 0; idx < buffer_lines.size(); ++idx) {
        result += buffer_lines[idx];
        if (idx < buffer_lines.size() - 1) {
            result += "\n";
        }
    }
    return result;
}

void TextArea::set_text(const std::string& text) {
    buffer_lines.clear();
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        buffer_lines.emplace_back(line);
    }
    if (buffer_lines.empty()) {
        buffer_lines.emplace_back("");
    }
    cursor_y   = 0;
    cursor_x   = 0;
    scroll_top = 0;
}

void TextArea::render() {
    if (plane == nullptr) {
        return;
    }

    const Style* cur_style_ptr = &style;
    if (disabled) {
        cur_style_ptr = &disabled_style;
    } else if (is_focused) {
        cur_style_ptr = &focused_style;
    }
    const Style& cur_style = *cur_style_ptr;
    cur_style.apply(plane);
    ncplane_erase(plane);
    draw_box(cur_style);

    int border_offset = cur_style.framed ? 1 : 0;
    int client_y = cur_style.pt + border_offset;
    int client_x = cur_style.pl + border_offset;

    int usable_h = height - cur_style.pt - cur_style.pb - (cur_style.framed ? 2 : 0);
    int usable_w = width  - cur_style.pl - cur_style.pr - (cur_style.framed ? 2 : 0);
    if (usable_h < 1) {
        usable_h = 1;
    }
    if (usable_w < 1) {
        usable_w = 1;
    }

    if (cursor_y < scroll_top) {
        scroll_top = cursor_y;
    } else if (cursor_y >= scroll_top + usable_h) {
        scroll_top = cursor_y - usable_h + 1;
    }

    bool empty = (buffer_lines.size() == 1 && buffer_lines[0].empty());
    if (empty && !is_focused && !placeholder.empty()) {
        ncplane_set_fg_rgb8(plane, 120, 120, 120);
        ncplane_putstr_yx(plane, client_y, client_x, placeholder.substr(0, usable_w).c_str());
        return;
    }

    for (int row = 0; row < usable_h; ++row) {
        int line_idx = scroll_top + row;
        if (line_idx >= static_cast<int>(buffer_lines.size())) {
            break;
        }

        const std::string& line = buffer_lines[line_idx];
        std::string visible = !line.empty() ? line.substr(0, usable_w) : "";
        ncplane_putstr_yx(plane, client_y + row, client_x, visible.c_str());

        if (is_focused && line_idx == cursor_y) {
            draw_cursor(plane, cur_style,
                        CursorCtx{client_y, client_x, usable_w, row, cursor_x}, line);
        }
    }
}

auto TextArea::handle_input(const ncinput& nc_input) -> bool {
    if (disabled) {
        return false;
    }
    if (nc_input.evtype == NCTYPE_RELEASE) {
        return false;
    }

    if (apply_motion_key(nc_input, cursor_x, cursor_y, buffer_lines)) {
        return true;
    }

    if (apply_edit_key(nc_input, cursor_x, cursor_y, buffer_lines)) {
        emit("change", get_text());
        return true;
    }
    return false;
}

} 
