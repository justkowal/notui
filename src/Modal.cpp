#include "notui/Modal.h"
#include "notui/HBox.h"
#include "notui/Spacer.h"

namespace notui {

Modal::Modal(std::string title, std::string message, std::function<void(bool)> callback) 
    : on_close(std::move(callback)) {
    fixed_height = 8;
    fixed_width = 50;
    is_overlay = true;
    
    style.bg({30, 30, 35}).fg({255, 255, 255}).frame(true, true);
    
    title_label = std::make_shared<Label>(std::move(title), Size{1, 40}, true);
    title_label->style.fg({80, 150, 255}).attr(NCSTYLE_BOLD);
    add_child(title_label);
    
    // message body label
    message_label = std::make_shared<Label>(std::move(message), Size{3, 46}, true);
    message_label->style.fg({200, 200, 200});
    add_child(message_label);
    
    auto btn_row = std::make_shared<HBox>();
    btn_row->fixed_height = 1;
    btn_row->main_axis_alignment = MainAxisAlignment::Center;
    
    ok_btn = std::make_shared<Button>("OK", [this]() {
        if (this->on_close != nullptr) {
            this->on_close(true);
        }
    });
    ok_btn->style.bg({40, 120, 60}).fg({255, 255, 255});
    btn_row->add_child(ok_btn);
    
    auto spacer = std::make_shared<Spacer>();
    spacer->fixed_width = 4;
    spacer->flex = 0;
    btn_row->add_child(spacer);
    
    cancel_btn = std::make_shared<Button>("Cancel", [this]() {
        if (this->on_close != nullptr) {
            this->on_close(false);
        }
    });
    cancel_btn->style.bg({120, 40, 40}).fg({255, 255, 255});
    btn_row->add_child(cancel_btn);
    
    add_child(btn_row);
}

void Modal::layout(struct ncplane* parent_plane, Point pos, Size size) {
    abs_y = pos.y; 
    abs_x = pos.x; 
    height = size.height; 
    width = size.width;
    
    if (backdrop_plane != nullptr) {
        ncplane_destroy(backdrop_plane);
        backdrop_plane = nullptr;
    }

    struct ncplane_options b_opts = {
        .y = 0, .x = 0,
        .rows = static_cast<unsigned>(size.height),
        .cols = static_cast<unsigned>(size.width),
        .userptr = this,
        .name = "modal_backdrop",
        .resizecb = nullptr,
        .flags = 0
    };
    backdrop_plane = ncplane_create(parent_plane, &b_opts);
    if (backdrop_plane != nullptr) {
        ncplane_move_top(backdrop_plane);
        
        // dim backdrop plane
        uint64_t b_channels = 0;
        ncchannels_set_bg_rgb8(&b_channels, 10, 10, 15);
        ncchannels_set_bg_alpha(&b_channels, NCALPHA_BLEND);
        ncchannels_set_fg_alpha(&b_channels, NCALPHA_TRANSPARENT);
        ncplane_set_base(backdrop_plane, "", 0, b_channels);
        ncplane_erase(backdrop_plane);
    }

    int center_y = (size.height - fixed_height) / 2;
    int center_x = (size.width - fixed_width) / 2;
    VBox::layout(backdrop_plane, Point{center_y, center_x}, Size{fixed_height, fixed_width});
}

void Modal::destroy_planes() {
    VBox::destroy_planes();
    if (backdrop_plane != nullptr) {
        ncplane_destroy(backdrop_plane);
        backdrop_plane = nullptr;
    }
}

} // namespace notui
