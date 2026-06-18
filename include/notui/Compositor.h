#pragma once

#include "notui/Widget.h"
#include "notui/FocusManager.h"
#include <memory>
#include <functional>

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

    void set_idle_callback(std::function<void()> callback) { idle_callback = std::move(callback); }
    void set_global_shortcut_handler(std::function<bool(const ncinput&)> handler) { global_shortcut_handler = std::move(handler); }

    [[nodiscard]] auto get_focus_manager() -> FocusManager& { return focus_manager; }
private:
    std::function<void()> idle_callback = nullptr;
    std::function<bool(const ncinput&)> global_shortcut_handler = nullptr;
};

} 
