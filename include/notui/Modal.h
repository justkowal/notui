#pragma once

#include "notui/VBox.h"
#include "notui/Label.h"
#include "notui/Button.h"
#include <string>
#include <functional>
#include <memory>

namespace notui {

struct Modal : public VBox {
protected:
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
    std::shared_ptr<Label> title_label;
    std::shared_ptr<Label> message_label;
    std::shared_ptr<Button> ok_btn;
    std::shared_ptr<Button> cancel_btn;
    struct ncplane* backdrop_plane = nullptr;
    std::function<void(bool)> on_close;
    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)

public:
    explicit Modal(std::string title, std::string message, std::function<void(bool)> callback);
    explicit Modal(std::string title, int width, int height);
    ~Modal() override = default;

    Modal(const Modal&) = delete;
    auto operator=(const Modal&) -> Modal& = delete;
    Modal(Modal&&) = delete;
    auto operator=(Modal&&) -> Modal& = delete;

    void layout(struct ncplane* parent_plane, Point pos, Size size) override;
    void destroy_planes() override;
    void raise_to_top() override;
};

} // namespace notui
