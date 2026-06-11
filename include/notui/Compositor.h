#pragma once

#include "notui/Widget.h"
#include "notui/FocusManager.h"
#include <memory>

struct notcurses;

namespace notui {

class Compositor {
private:
    struct notcurses* nc = nullptr;
    std::shared_ptr<Widget> root;
    FocusManager focus_manager;
    bool running = true;

public:
    explicit Compositor(std::shared_ptr<Widget> root_widget);
    ~Compositor();

    Compositor(const Compositor&) = delete;
    auto operator=(const Compositor&) -> Compositor& = delete;
    Compositor(Compositor&&) = delete;
    auto operator=(Compositor&&) -> Compositor& = delete;

    void quit();
    void trigger_layout();
    void run();

    [[nodiscard]] auto get_focus_manager() -> FocusManager& { return focus_manager; }
};

} // namespace notui
