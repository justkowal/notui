#pragma once

#include "notui/Widget.h"
#include <vector>
#include <memory>

namespace notui {

class VBox : public Widget {
public:
    VBox(struct ncplane* parent, int y, int x, int height, int width);

    auto addChild(std::unique_ptr<Widget> child) -> void;
    auto setFocusManager(FocusManager* manager) -> void override;
    auto collectFocusable(std::vector<Widget*>& focusables) -> void override;

    auto resizeAndMove(int y, int x, int height, int width) -> void override;
    auto render() -> void override;
    auto handleInput(const ncinput& input) -> bool override;
    auto acceptsFocus() const -> bool override { return false; }

private:
    std::vector<std::unique_ptr<Widget>> children_;
    auto recalculateLayout() -> void;
};

} // namespace notui