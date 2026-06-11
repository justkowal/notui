#include "notui/Dropdown.h"
#include <algorithm>

namespace notui {

Dropdown::Dropdown(std::vector<std::string> opts, int default_idx) 
    : options(std::move(opts)), selected_idx(default_idx) {
    fixed_height = 1;
    fixed_width = 25;
    focusable = true;
    
    style.bg({35, 35, 45}).fg({220, 220, 220}).pad({0, 1, 0, 1});
    focused_style.bg({50, 70, 90}).fg({255, 255, 255}).pad({0, 1, 0, 1}).attr(NCSTYLE_BOLD);
}

auto Dropdown::get_selected_value() const -> std::string {
    if (selected_idx >= 0 && selected_idx < static_cast<int>(options.size())) {
        return options[selected_idx];
    }
    return "";
}

void Dropdown::render() {
    const Style& dropdown_style = is_focused ? focused_style : style;
    
    if (expanded) {
        // temporarily grow plane to show list
        ncplane_resize(plane, 0, 0, 0, 0, 0, 0, 1 + static_cast<int>(options.size()), width);
        ncplane_move_top(plane);
    } else {
        // shrink back to single line
        ncplane_resize(plane, 0, 0, 0, 0, 0, 0, 1, width);
    }
    
    dropdown_style.apply(plane);
    ncplane_erase(plane);
    
    // draw collapsed header
    std::string disp = get_selected_value();
    if (disp.empty()) {
        disp = "-- Select --";
    }
    std::string arrow = expanded ? "▲" : "▼";
    std::string header_text = " " + disp;
    
    int space_left = width - static_cast<int>(header_text.length()) - 3;
    if (space_left > 0) {
        header_text += std::string(space_left, ' ');
    }
    header_text += " " + arrow;
    ncplane_putstr_yx(plane, 0, 0, header_text.c_str());

    if (expanded) {
        // draw options
        for (size_t i = 0; i < options.size(); ++i) {
            int row_y = 1 + static_cast<int>(i);
            bool opt_selected = (static_cast<int>(i) == selected_idx);
            
            // highlight selected option
            if (opt_selected) {
                ncplane_set_bg_rgb8(plane, 80, 150, 255);
                ncplane_set_fg_rgb8(plane, 0, 0, 0);
            } else {
                ncplane_set_bg_rgb8(plane, 40, 40, 50);
                ncplane_set_fg_rgb8(plane, 200, 200, 200);
            }
            
            ncplane_putstr_yx(plane, row_y, 0, std::string(width, ' ').c_str());
            
            std::string opt_text = (opt_selected ? " > " : "   ") + options[i];
            if (static_cast<int>(opt_text.length()) > width) {
                opt_text = opt_text.substr(0, width - 3) + "...";
            }
            ncplane_putstr_yx(plane, row_y, 0, opt_text.c_str());
        }
    }

    // reset debouncing flag
    just_opened = false;
}

auto Dropdown::handle_input(const ncinput& nc_input) -> bool { // NOLINT(readability-function-cognitive-complexity)
    if (nc_input.evtype == NCTYPE_RELEASE) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    bool was_expanded = expanded;

    if (!was_expanded) {
        // toggle open on enter/space/click
        if ((nc_input.id == NCKEY_ENTER || nc_input.id == 13 || nc_input.id == 10 || nc_input.id == ' ' || nc_input.id == NCKEY_BUTTON1) && 
            (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            expanded = true;
            just_opened = true;
            open_time = now;
            return true;
        }
    } else {
        // debounce: ignore events within cooldown
        if (just_opened) {
            return true;
        }
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - open_time).count();
        if (elapsed_ms < 150) {
            return true;
        }

        // handle events when open
        if (nc_input.id == NCKEY_UP && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            if (selected_idx > 0) {
                selected_idx--;
            } else {
                selected_idx = static_cast<int>(options.size()) - 1;
            }
            return true;
        }
        if (nc_input.id == NCKEY_DOWN && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            if (selected_idx < static_cast<int>(options.size()) - 1) {
                selected_idx++;
            } else {
                selected_idx = 0;
            }
            return true;
        }
        if ((nc_input.id == NCKEY_ENTER || nc_input.id == 13 || nc_input.id == 10 || nc_input.id == ' ') && 
            (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            expanded = false;
            emit("change", selected_idx);
            if (on_select != nullptr) {
                on_select(selected_idx);
            }
            return true;
        }
        if (nc_input.id == NCKEY_ESC && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            expanded = false;
            return true;
        }
        if (nc_input.id == NCKEY_BUTTON1 && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            int plane_y = ncplane_abs_y(plane);
            int rel_y = nc_input.y - plane_y;
            if (rel_y > 0 && rel_y <= static_cast<int>(options.size())) {
                selected_idx = rel_y - 1;
                expanded = false;
                emit("change", selected_idx);
                if (on_select != nullptr) {
                    on_select(selected_idx);
                }
            } else {
                expanded = false;
            }
            return true;
        }
    }
    return false;
}

void Dropdown::on_blur() {
    Widget::on_blur();
    expanded = false;
}

auto Dropdown::get_widget_at(int pos_y, int pos_x) -> Widget* {
    if (plane == nullptr) {
        return nullptr;
    }
    int plane_y = ncplane_abs_y(plane);
    int plane_x = ncplane_abs_x(plane);
    int current_h = expanded ? (1 + static_cast<int>(options.size())) : height;
    if (pos_y >= plane_y && pos_y < plane_y + current_h && pos_x >= plane_x && pos_x < plane_x + width) {
        return this;
    }
    return nullptr;
}

void Dropdown::raise_to_top() {
    Widget::raise_to_top();
}

auto Dropdown::is_active_overlay() -> bool {
    return expanded;
}

} // namespace notui
