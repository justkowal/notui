#pragma once

#include "notui/Widget.h"

namespace notui {

class Spacer : public Widget {
public:
    Spacer();
    void render() override;
};

} // namespace notui