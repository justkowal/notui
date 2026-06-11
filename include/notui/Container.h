#pragma once

#include "notui/Widget.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace notui {

enum class MainAxisAlignment : uint8_t {
    Start,
    Center,
    End,
    SpaceBetween,
    SpaceAround
};

enum class CrossAxisAlignment : uint8_t {
    Start,
    Center,
    End,
    Stretch
};

struct Container : public Widget {
    std::vector<std::shared_ptr<Widget>> children;

    MainAxisAlignment main_axis_alignment = MainAxisAlignment::Start;
    CrossAxisAlignment cross_axis_alignment = CrossAxisAlignment::Stretch;

    void add_child(std::shared_ptr<Widget> child);
    void remove_child(const std::shared_ptr<Widget>& child);
    auto get_children() const -> const std::vector<std::shared_ptr<Widget>>&;

    void destroy_planes() override;
    void render() override;
    auto contains_focus() -> bool override;
    auto get_widget_at(int pos_y, int pos_x) -> Widget* override;
};

} // namespace notui
