#pragma once

#include "notui/Widget.h"
#include <string>

namespace notui {

class Checkbox : public Widget {
private:
    std::string label;
    bool checked = false;

public:
    explicit Checkbox(std::string label_text, bool default_checked = false);
    ~Checkbox() override = default;

    Checkbox(const Checkbox&) = delete;
    auto operator=(const Checkbox&) -> Checkbox& = delete;
    Checkbox(Checkbox&&) = delete;
    auto operator=(Checkbox&&) -> Checkbox& = delete;

    void render() override;
    auto handle_input(const ncinput& nc_input) -> bool override;

    [[nodiscard]] auto is_checked() const -> bool { return checked; }
    void set_checked(bool checked_state);
};

} // namespace notui
