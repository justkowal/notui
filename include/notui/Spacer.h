#pragma once

#include "notui/Widget.h"

namespace notui {

class Spacer : public Widget {
public:
    explicit Spacer(struct ncplane* parent)
        : Widget(parent, 0, 0, 1, 1) {
        // A spacer's entire purpose is to greedily consume leftover space
        setHeightPolicy(SizeMode::Expand);
        setWidthPolicy(SizeMode::Expand);
    }

    auto render() -> void override {
        // Invisible widget; just clear the plane
        ncplane_erase(plane_);
    }

    auto handleInput(const ncinput& /*input*/) -> bool override {
        // Spacers never intercept mouse clicks or keystrokes
        return false; 
    }
};

} // namespace notui