#include "notui/Compositor.h"
#include "notui/Container.h"
#include "notui/Button.h"
#include <notcurses/notcurses.h>
#include <clocale>
#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <functional>

namespace notui {

namespace {
void raise_subtree(Widget* widget) {
    if (widget == nullptr) {
        return;
    }
    widget->raise_to_top();
    if (auto* container = dynamic_cast<Container*>(widget)) {
        for (const auto& child : container->get_children()) {
            raise_subtree(child.get());
        }
    }
}

void raise_overlays(Widget* widget) {
    if (widget == nullptr) {
        return;
    }
    bool is_active = false;
    if (auto* overlay = dynamic_cast<IOverlay*>(widget)) {
        is_active = overlay->is_active_overlay();
    } else {
        is_active = widget->is_active_overlay();
    }
    
    if (is_active) {
        raise_subtree(widget);
    }
    if (auto* container = dynamic_cast<Container*>(widget)) {
        for (const auto& child : container->get_children()) {
            raise_overlays(child.get());
        }
    }
}

auto get_terminal_size(int& rows, int& cols) -> bool {
    struct winsize w;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        rows = w.ws_row;
        cols = w.ws_col;
        return true;
    }
    return false;
}
} // namespace

Compositor::Compositor(std::shared_ptr<Widget> root_widget) : root(std::move(root_widget)) {
    std::setlocale(LC_ALL, "");
    struct notcurses_options opts = {
        .flags = NCOPTION_SUPPRESS_BANNERS
    };
    nc = notcurses_core_init(&opts, stdout);
    notcurses_mice_enable(nc, NCMICE_DRAG_EVENT);

    if (root != nullptr) {
        focus_manager.rebuild(*root);
    }
}

Compositor::~Compositor() {
    if (root != nullptr) {
        root->destroy_planes();
    }
    if (nc != nullptr) {
        notcurses_stop(nc);
    }
}

void Compositor::quit() { 
    running = false; 
}

void Compositor::trigger_layout() {
    if (nc == nullptr || root == nullptr) {
        return;
    }
    unsigned rows = 0;
    unsigned cols = 0;
    notcurses_refresh(nc, &rows, &cols);
    root->destroy_planes(); 
    root->layout(notcurses_stdplane(nc), Point{0, 0}, Size{static_cast<int>(rows), static_cast<int>(cols)});
    
    focus_manager.rebuild(*root);
}

void Compositor::run() { // NOLINT(readability-function-cognitive-complexity)
    trigger_layout();

    int last_rows = 0;
    int last_cols = 0;
    get_terminal_size(last_rows, last_cols);

    ncinput nc_input;
    while (running) {
        int current_rows = 0;
        int current_cols = 0;
        if (get_terminal_size(current_rows, current_cols)) {
            if (current_rows != last_rows || current_cols != last_cols) {
                last_rows = current_rows;
                last_cols = current_cols;
                trigger_layout();
            }
        }

        if (idle_callback) {
            idle_callback();
        }
        if (root != nullptr) {
            root->render();
            raise_overlays(root.get());
        }
        notcurses_render(nc);

        struct timespec timeout_spec = {0, 10000000}; 
        uint32_t result = notcurses_get(nc, &timeout_spec, &nc_input);

        if (result == static_cast<uint32_t>(-1)) {
            continue;
        }

        if (global_shortcut_handler && global_shortcut_handler(nc_input)) {
            continue;
        }

        // keep widgets alive during event loop
        std::vector<std::shared_ptr<Widget>> keep_alive;
        if (root != nullptr) {
            std::function<void(const std::shared_ptr<Widget>&)> collect;
            collect = [&](const std::shared_ptr<Widget>& widget) {
                if (widget == nullptr) {
                    return;
                }
                keep_alive.emplace_back(widget);
                auto container = std::dynamic_pointer_cast<Container>(widget);
                if (container != nullptr) {
                    for (const auto& child : container->get_children()) {
                        collect(child);
                    }
                }
            };
            collect(root);
        }
        
        if (nc_input.id == static_cast<uint32_t>(NCKEY_RESIZE)) {
            trigger_layout();
            continue;
        }

        if (nc_input.id == static_cast<uint32_t>(NCKEY_ESC)) {
            break;
        }

        if (nc_input.id == static_cast<uint32_t>(NCKEY_TAB) && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            const bool shift_pressed = nc_input.shift || ((nc_input.modifiers & NCKEY_MOD_SHIFT) != 0);
            if (shift_pressed) {
                focus_manager.focus_prev();
            } else {
                focus_manager.focus_next();
            }
            // force layout if tabbed into scrollarea
            trigger_layout(); 
            continue;
        }

        bool is_mouse = (nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON1) || nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON2) || nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON3) || 
                         nc_input.id == static_cast<uint32_t>(NCKEY_SCROLL_UP) || nc_input.id == static_cast<uint32_t>(NCKEY_SCROLL_DOWN) || nc_input.id == static_cast<uint32_t>(NCKEY_MOTION));
        
        bool handled = false;

        // spatial routing for mouse
        if (is_mouse && root != nullptr) {
            Widget* active_overlay = FocusManager::get_active_overlay(root.get());
            Widget* focused = focus_manager.focusedWidget();
            Widget* hit = nullptr;
            if (focused != nullptr) {
                hit = focused->get_widget_at(nc_input.y, nc_input.x);
            }
            if (hit == nullptr) {
                hit = root->get_widget_at(nc_input.y, nc_input.x);
            }
            if (hit != nullptr) {
                // block interaction if outside active overlay
                if (active_overlay != nullptr) {
                    bool inside_overlay = false;
                    Widget* temp = hit;
                    while (temp != nullptr) {
                        if (temp == active_overlay) {
                            inside_overlay = true;
                            break;
                        }
                        temp = temp->parent;
                    }
                    if (!inside_overlay) {
                        continue;
                    }
                }

                if (nc_input.id == static_cast<uint32_t>(NCKEY_BUTTON1) && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
                    Widget* target = hit;
                    while (target != nullptr && !target->focusable) {
                        target = target->parent;
                    }
                    if (target != nullptr) {
                        focus_manager.set_focus(target);
                    }
                }
                
                // bubble event upwards
                Widget* curr = hit;
                while (curr != nullptr) {
                    Widget* next = curr->parent;
                    if (curr->handle_input(nc_input)) { 
                        handled = true; 
                        if (dynamic_cast<Button*>(curr) != nullptr) {
                            focus_manager.set_focus(-1);
                        }
                        break; 
                    }
                    curr = next;
                }
            }
        }
        if (!handled && !is_mouse) {
            Widget* focused_widget = focus_manager.focusedWidget();
            if (focused_widget != nullptr && focused_widget->plane != nullptr) {
                handled = focused_widget->handle_input(nc_input); // don't send keystrokes to invisible widgets
            }
        }

        if (!handled && (nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_UNKNOWN)) {
            if (nc_input.id == static_cast<uint32_t>(NCKEY_UP) || nc_input.id == static_cast<uint32_t>(NCKEY_DOWN) || nc_input.id == static_cast<uint32_t>(NCKEY_LEFT) || nc_input.id == static_cast<uint32_t>(NCKEY_RIGHT)) {
                focus_manager.handle_directional_focus(static_cast<int>(nc_input.id));
            }
        }
    }
}

} // namespace notui
