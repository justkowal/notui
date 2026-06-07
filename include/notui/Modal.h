#pragma once

#include "notui/Widget.h"
#include <memory>
#include <vector>
#include <string>

namespace notui {

class Modal : public Widget {
public:
    Modal(struct ncplane* parent, int pos_y, int pos_x, int height, int width, std::string title = "");

    // Inject any layout or widget to be displayed inside the modal window
    auto setContent(std::unique_ptr<Widget> content) -> void;
    auto setFocusManager(FocusManager* manager) -> void override;
    auto collectFocusable(std::vector<Widget*>& focusables) -> void override;

    auto render() -> void override;
    
    // Returns true if the modal trapped the input (preventing background clicks)
    auto handleInput(const ncinput& input) -> bool override;

    // Forces this window to render above all other UI elements
    auto bringToFront() -> void;

private:
    std::string title_;
    std::unique_ptr<Widget> content_;

    // Drag tracking state
    bool is_dragging_ = false;
    int drag_start_y_ = 0;
    int drag_start_x_ = 0;
    int plane_start_y_ = 0;
    int plane_start_x_ = 0;
};

} // namespace notui