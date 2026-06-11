#pragma once

#include "notui/Widget.h"
#include <string>

namespace notui {

class Canvas : public Widget {
private:
    std::string image_path;
    struct ncvisual* visual = nullptr;
    ncscale_e scaling_mode = NCSCALE_STRETCH;

    struct ncplane* visual_plane = nullptr;

    void load_visual();
    void clear_visual();

public:
    explicit Canvas(std::string path = "", Size fixed_size = {});
    ~Canvas() override;

    Canvas(const Canvas&) = delete;
    auto operator=(const Canvas&) -> Canvas& = delete;
    Canvas(Canvas&&) = delete;
    auto operator=(Canvas&&) -> Canvas& = delete;

    void destroy_planes() override;
    void layout(struct ncplane* parent_plane, Point pos, Size size) override;
    void render() override;
    
    void set_image_path(const std::string& path);
    [[nodiscard]] auto get_image_path() const -> std::string { return image_path; }

    void set_framebuffer(const uint32_t* rgba, int rows, int cols, int rowstride = 0);

    void set_scaling(ncscale_e scaling) { scaling_mode = scaling; }
    [[nodiscard]] auto get_scaling() const -> ncscale_e { return scaling_mode; }
};

} // namespace notui
