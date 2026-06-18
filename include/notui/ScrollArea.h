#pragma once

#include "notui/Container.h"

namespace notui {

struct ScrollArea : public Container {
private:
    int scroll_y = 0;
    struct ncplane* overlay_plane = nullptr;

public:
    ScrollArea();
    ~ScrollArea() override = default;

    ScrollArea(const ScrollArea&) = delete;
    auto operator=(const ScrollArea&) -> ScrollArea& = delete;
    ScrollArea(ScrollArea&&) = delete;
    auto operator=(ScrollArea&&) -> ScrollArea& = delete;

    void destroy_planes() override;
    void layout(struct ncplane* parent_plane, Point pos, Size size) override;
    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;
    auto get_widget_at(int pos_y, int pos_x) -> Widget* override;
};

} 
