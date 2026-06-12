#include "notui/VBox.h"
#include <algorithm>

namespace notui {

namespace {
bool is_overlay_child(const std::shared_ptr<Widget>& child) {
    if (child == nullptr) {
        return false;
    }
    if (auto* overlay = dynamic_cast<IOverlay*>(child.get())) {
        return overlay->is_active_overlay();
    }
    return child->is_overlay;
}
} // namespace

void VBox::layout(struct ncplane* parent_plane, Point pos, Size size) { // NOLINT(readability-function-cognitive-complexity)
    Widget::layout(parent_plane, pos, size);
    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, size.height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, size.width - style.pl - style.pr - (style.framed ? 2 : 0));

    int total_flex = 0;
    int used_height = 0;
    for (auto& child : children) {
        if (child != nullptr && !is_overlay_child(child)) {
            if (child->flex > 0) {
                total_flex += child->flex;
            } else {
                used_height += child->fixed_height;
            }
        }
    }

    int available_flex_height = std::max(0, usable_h - used_height);
    int total_content_height = 0;
    std::vector<int> child_heights(children.size(), 0);

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] != nullptr) {
            if (is_overlay_child(children[i])) {
                child_heights[i] = 0;
            } else if (children[i]->flex > 0) {
                child_heights[i] = total_flex > 0 ? (available_flex_height * children[i]->flex) / total_flex : 0;
                total_content_height += child_heights[i];
            } else {
                child_heights[i] = children[i]->fixed_height;
                total_content_height += child_heights[i];
            }
        }
    }

    int remaining_space = std::max(0, usable_h - total_content_height);
    int current_y = style.pt + border_offset;
    int gap = 0;

    int non_overlay_count = 0;
    for (auto& child : children) {
        if (child != nullptr && !is_overlay_child(child)) {
            non_overlay_count++;
        }
    }

    if (remaining_space > 0) {
        if (main_axis_alignment == MainAxisAlignment::Center) {
            current_y += remaining_space / 2;
        } else if (main_axis_alignment == MainAxisAlignment::End) {
            current_y += remaining_space;
        } else if (main_axis_alignment == MainAxisAlignment::SpaceBetween && non_overlay_count > 1) {
            gap = remaining_space / (non_overlay_count - 1);
        } else if (main_axis_alignment == MainAxisAlignment::SpaceAround && non_overlay_count > 0) {
            gap = remaining_space / (non_overlay_count + 1);
            current_y += gap;
        }
    }

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] != nullptr) {
            if (is_overlay_child(children[i])) {
                children[i]->layout(plane, Point{0, 0}, Size{height, width});
            } else {
                int child_w = usable_w;
                int child_x = style.pl + border_offset;
                
                if (cross_axis_alignment != CrossAxisAlignment::Stretch) {
                    int desired_w = children[i]->fixed_width > 0 ? children[i]->fixed_width : usable_w;
                    child_w = std::min(usable_w, desired_w);
                    if (cross_axis_alignment == CrossAxisAlignment::Center) {
                        child_x += (usable_w - child_w) / 2;
                    } else if (cross_axis_alignment == CrossAxisAlignment::End) {
                        child_x += (usable_w - child_w);
                    }
                }
                
                children[i]->layout(plane, Point{current_y, child_x}, Size{child_heights[i], child_w});
                current_y += child_heights[i] + gap;
            }
        }
    }
}

} // namespace notui