#include "notui/Checkbox.h"

namespace notui {

Checkbox::Checkbox(std::string label_text, bool default_checked) 
    : label(std::move(label_text)), checked(default_checked) {
    fixed_height = 1;
    fixed_width = static_cast<int>(label.length()) + 6; // "[ ] " + label
    focusable = true;
    
    style.transparent(true).fg({200, 200, 200}).pad({0, 1, 0, 1});
    focused_style.bg({45, 50, 65}).fg({255, 255, 255}).pad({0, 1, 0, 1}).attr(NCSTYLE_BOLD);
}

void Checkbox::render() {
    const Style& checkbox_style = is_focused ? focused_style : style;
    checkbox_style.apply(plane);
    ncplane_erase(plane);
    
    std::string box = checked ? "[x]" : "[ ]";
    std::string text = " " + box + " " + label;
    ncplane_putstr_yx(plane, 0, 0, text.c_str());
}

auto Checkbox::handle_input(const ncinput& nc_input) -> bool {
    if (nc_input.evtype == NCTYPE_RELEASE) {
        return false;
    }
    
    if ((nc_input.id == NCKEY_ENTER || nc_input.id == 13 || nc_input.id == 10 || nc_input.id == ' ' || nc_input.id == NCKEY_BUTTON1) && 
        (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
        checked = !checked;
        emit("change", checked);
        return true;
    }
    return false;
}

void Checkbox::set_checked(bool checked_state) {
    if (checked != checked_state) {
        checked = checked_state;
        emit("change", checked);
    }
}

} // namespace notui
