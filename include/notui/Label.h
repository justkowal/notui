#pragma once

#include "notui/Widget.h"
#include <string>

namespace notui {

class Label : public Widget {
private:
    std::string text;
    bool centered;
public:
    explicit Label(std::string label_text, Size fixed_size = {}, bool center = false);
    ~Label() override = default;

    Label(const Label&) = delete;
    auto operator=(const Label&) -> Label& = delete;
    Label(Label&&) = delete;
    auto operator=(Label&&) -> Label& = delete;
    
    void set_text(const std::string& new_text);
    void render() override;
};

} // namespace notui