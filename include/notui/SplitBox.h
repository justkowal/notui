#pragma once

#include "notui/Container.h"

namespace notui {

struct SplitBox : public Container {
public:
    enum class Orientation : uint8_t {
        Horizontal,
        Vertical
    };

    explicit SplitBox(Orientation orientation = Orientation::Horizontal, double default_split_percent = 0.5);
    ~SplitBox() override = default;

    SplitBox(const SplitBox&) = delete;
    auto operator=(const SplitBox&) -> SplitBox& = delete;
    SplitBox(SplitBox&&) = delete;
    auto operator=(SplitBox&&) -> SplitBox& = delete;

    void layout(struct ncplane* parent_plane, Point pos, Size size) override;
    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;
    auto get_widget_at(int pos_y, int pos_x) -> Widget* override;

    void set_split_enabled(bool enabled);
    [[nodiscard]] auto is_split_enabled() const -> bool;

    void set_split_percent(double percent);
    [[nodiscard]] auto get_split_percent() const -> double;

private:
    Orientation orientation_;
    double split_percent_;
    bool split_enabled_{false};
    bool is_dragging_{false};
};

} 
