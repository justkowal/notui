#pragma once

#include "notui/Container.h"

namespace notui {

struct VBox : public Container {
public:
    void layout(struct ncplane* parent_plane, Point pos, Size size) override;
};

} // namespace notui