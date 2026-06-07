#pragma once

#include "notui/Widget.h"
#include <memory>
#include <vector>

namespace notui {

enum class SplitDirection { Horizontal, Vertical };

class SplitContainer : public Widget {
public:
    SplitContainer(struct ncplane* parent, SplitDirection direction = SplitDirection::Horizontal);

    auto setFirst(std::unique_ptr<Widget> first) -> void;
    auto setSecond(std::unique_ptr<Widget> second) -> void;
    auto setFocusManager(FocusManager* manager) -> void override;
    auto collectFocusable(std::vector<Widget*>& focusables) -> void override;

    auto resizeAndMove(int y, int x, int height, int width) -> void override;
    auto render() -> void override;
    auto handleInput(const ncinput& input) -> bool override;

    auto setSplitRatio(float ratio) -> void; // e.g., 0.5 for 50/50

private:
    SplitDirection direction_;
    std::unique_ptr<Widget> first_;
    std::unique_ptr<Widget> second_;

    float split_ratio_ = 0.5f;
    int split_pos_ = 0; // Calculated absolute cell position of the divider

    bool is_dragging_ = false;

    auto recalculateLayout() -> void;
};

} // namespace notui