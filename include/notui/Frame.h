#pragma once

#include "notui/Widget.h"
#include <memory>
#include <vector>
#include <string>

namespace notui {

class Frame : public Widget {
public:
    Frame(struct ncplane* parent, int y, int x, int height, int width, std::string title = "");

    // A Frame holds exactly one child widget. 
    auto setChild(std::unique_ptr<Widget> child) -> void;
    auto setFocusManager(FocusManager* manager) -> void override;
    auto collectFocusable(std::vector<Widget*>& focusables) -> void override;

    auto resizeAndMove(int y, int x, int height, int width) -> void override;
    auto render() -> void override;
    auto handleInput(const ncinput& input) -> bool override;

private:
    std::unique_ptr<Widget> child_;
    std::string title_;
};

} // namespace notui