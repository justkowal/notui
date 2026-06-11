#pragma once

#include "notui/Widget.h"
#include <string>
#include <functional>


namespace notui {

struct Button : public Widget {
public:
    std::string label;
    std::function<void()> on_click; 
    
    Button(std::string label_text, std::function<void()> click_callback);
    ~Button() override = default;

    Button(const Button&) = delete;
    auto operator=(const Button&) -> Button& = delete;
    Button(Button&&) = delete;
    auto operator=(Button&&) -> Button& = delete;

    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;
};

} // namespace notui