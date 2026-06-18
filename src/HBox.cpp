#include "notui/HBox.h"
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
} 

void HBox::layout(struct ncplane* parent_plane, Point pos, Size size) { // NOLINT(readability-function-cognitive-complexity)
    Widget::layout(parent_plane, pos, size);
    int border_offset = style.framed ? 1 : 0;
    int usable_h = std::max(0, size.height - style.pt - style.pb - (style.framed ? 2 : 0));
    int usable_w = std::max(0, size.width - style.pl - style.pr - (style.framed ? 2 : 0));

    int total_flex = 0;
    int used_width = 0;
    for (auto& child : children) {
        if (child != nullptr && !is_overlay_child(child)) {
            if (child->flex > 0) {
                total_flex += child->flex;
            } else {
                used_width += child->fixed_width;
            }
        }
    }

    int available_flex_width = std::max(0, usable_w - used_width);
    int total_content_width = 0;
    std::vector<int> child_widths(children.size(), 0);

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] != nullptr) {
            if (is_overlay_child(children[i])) {
                child_widths[i] = 0;
            } else if (children[i]->flex > 0) {
                child_widths[i] = total_flex > 0 ? (available_flex_width * children[i]->flex) / total_flex : 0;
                total_content_width += child_widths[i];
            } else {
                child_widths[i] = children[i]->fixed_width;
                total_content_width += child_widths[i];
            }
        }
    }

    int remaining_space = std::max(0, usable_w - total_content_width);
    int current_x = style.pl + border_offset;
    int gap = 0;

    int non_overlay_count = 0;
    for (auto& child : children) {
        if (child != nullptr && !is_overlay_child(child)) {
            non_overlay_count++;
        }
    }

    if (remaining_space > 0) {
        if (main_axis_alignment == MainAxisAlignment::Center) {
            current_x += remaining_space / 2;
        } else if (main_axis_alignment == MainAxisAlignment::End) {
            current_x += remaining_space;
        } else if (main_axis_alignment == MainAxisAlignment::SpaceBetween && non_overlay_count > 1) {
            gap = remaining_space / (non_overlay_count - 1);
        } else if (main_axis_alignment == MainAxisAlignment::SpaceAround && non_overlay_count > 0) {
            gap = remaining_space / (non_overlay_count + 1);
            current_x += gap;
        }
    }

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i] != nullptr) {
            if (is_overlay_child(children[i])) {
                children[i]->layout(plane, Point{0, 0}, Size{height, width});
            } else {
                int child_h = usable_h;
                int child_y = style.pt + border_offset;
                
                if (cross_axis_alignment != CrossAxisAlignment::Stretch) {
                    int desired_h = children[i]->fixed_height > 0 ? children[i]->fixed_height : usable_h;
                    child_h = std::min(usable_h, desired_h);
                    if (cross_axis_alignment == CrossAxisAlignment::Center) {
                        child_y += (usable_h - child_h) / 2;
                    } else if (cross_axis_alignment == CrossAxisAlignment::End) {
                        child_y += (usable_h - child_h);
                    }
                }
                
                children[i]->layout(plane, Point{child_y, current_x}, Size{child_h, child_widths[i]});
                current_x += child_widths[i] + gap;
            }
        }
    }
}

} 