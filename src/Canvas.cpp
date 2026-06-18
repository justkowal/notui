#include "notui/Canvas.h"

namespace notui {

Canvas::Canvas(std::string path, Size fixed_size) 
    : image_path(std::move(path)) {
    fixed_height = fixed_size.height;
    fixed_width = fixed_size.width;
    focusable = false;
    style.transparent(true);
    
    if (!image_path.empty()) {
        load_visual();
    }
}

Canvas::~Canvas() {
    clear_visual();
    if (visual_plane != nullptr) {
        ncplane_destroy(visual_plane);
        visual_plane = nullptr;
    }
}

void Canvas::destroy_planes() {
    if (visual_plane != nullptr) {
        ncplane_destroy(visual_plane);
        visual_plane = nullptr;
    }
    Widget::destroy_planes();
}

void Canvas::layout(struct ncplane* parent_plane, Point pos, Size size) {
    Widget::layout(parent_plane, pos, size);
    
    if (visual_plane != nullptr) {
        ncplane_destroy(visual_plane);
        visual_plane = nullptr;
    }
    
    if (plane == nullptr) {
        return;
    }
    
    int border_offset = style.framed ? 1 : 0;
    int client_y = style.pt + border_offset;
    int client_x = style.pl + border_offset;
    int inner_h = size.height - style.pt - style.pb - (style.framed ? 2 : 0);
    int inner_w = size.width - style.pl - style.pr - (style.framed ? 2 : 0);
    
    if (inner_h > 0 && inner_w > 0) {
        struct ncplane_options opts = {};
        opts.y = client_y;
        opts.x = client_x;
        opts.rows = static_cast<unsigned>(inner_h);
        opts.cols = static_cast<unsigned>(inner_w);
        opts.userptr = this;
        opts.name = "canvas_visual";
        opts.resizecb = nullptr;
        opts.flags = 0;
        visual_plane = ncplane_create(plane, &opts);
    }
}

void Canvas::load_visual() {
    clear_visual();
    if (!image_path.empty()) {
        visual = ncvisual_from_file(image_path.c_str());
    }
}

void Canvas::clear_visual() {
    if (visual != nullptr) {
        ncvisual_destroy(visual);
        visual = nullptr;
    }
}

void Canvas::set_image_path(const std::string& path) {
    if (image_path != path) {
        image_path = path;
        load_visual();
    }
}

void Canvas::set_framebuffer(const uint32_t* rgba_data, int rows, int cols, int rowstride) {
    clear_visual();
    image_path = "";
    if (rgba_data != nullptr && rows > 0 && cols > 0) {
        if (rowstride == 0) {
            rowstride = cols * 4;
        }
        visual = ncvisual_from_rgba(rgba_data, rows, rowstride, cols);
    }
}

void Canvas::render() {
    if (plane == nullptr) {
        return;
    }

    ncplane_erase(plane);

    style.apply(plane);
    draw_box(style);

    if (visual != nullptr && visual_plane != nullptr) {
        ncplane_erase(visual_plane);
        
        struct ncvisual_options vopts = {};
        vopts.n = visual_plane;
        vopts.scaling = scaling_mode;
        vopts.blitter = NCBLIT_DEFAULT;
        
        struct notcurses* nc_context = ncplane_notcurses(visual_plane);
        if (nc_context != nullptr) {
            ncvisual_blit(nc_context, visual, &vopts);
        } else {
            ncplane_putstr_yx(visual_plane, 0, 0, "No Notcurses Ctx");
        }
    } else if (visual == nullptr) {
        int border_offset = style.framed ? 1 : 0;
        int client_y = style.pt + border_offset;
        int client_x = style.pl + border_offset;
        int inner_h = height - style.pt - style.pb - (style.framed ? 2 : 0);
        int inner_w = width - style.pl - style.pr - (style.framed ? 2 : 0);

        std::string err_msg = "No Image: " + image_path;
        if (static_cast<int>(err_msg.length()) > inner_w) {
            err_msg = err_msg.substr(0, std::max(0, inner_w - 3)) + "...";
        }
        int text_y = client_y + inner_h / 2;
        int text_x = client_x + std::max(0, (inner_w - static_cast<int>(err_msg.length())) / 2);
        ncplane_putstr_yx(plane, text_y, text_x, err_msg.c_str());
    }
}

} 
