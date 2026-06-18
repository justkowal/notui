#pragma once

#include "notui/Widget.h"
#include <vector>
#include <string>
#include <functional>
#include <chrono>

namespace notui {

struct Dropdown : public Widget {
private:
    std::vector<std::string> options;
    int selected_idx = 0;
    bool expanded = false;
    bool just_opened = false;
    std::chrono::steady_clock::time_point open_time;
    std::chrono::steady_clock::time_point close_time;
    std::function<void(int)> on_select;

public:
    explicit Dropdown(std::vector<std::string> opts, int default_idx = 0);
    ~Dropdown() override = default;

    Dropdown(const Dropdown&) = delete;
    auto operator=(const Dropdown&) -> Dropdown& = delete;
    Dropdown(Dropdown&&) = delete;
    auto operator=(Dropdown&&) -> Dropdown& = delete;

    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;
    void on_blur() override;
    auto get_widget_at(int pos_y, int pos_x) -> Widget* override;
    
    void raise_to_top() override;
    auto is_active_overlay() -> bool override;

    [[nodiscard]] auto get_selected_index() const -> int { return selected_idx; }
    [[nodiscard]] auto get_selected_value() const -> std::string;
};

} 
