#pragma once

#include <notcurses/notcurses.h>

#include <vector>

#include "notui/Widget.h"

namespace notui {

class FocusManager {
public:
  enum class Direction {
    Up,
    Down,
    Left,
    Right,
  };

  auto rebuild(Widget& root) -> void;
  auto focusedWidget() const -> Widget*;
  auto focusWidget(Widget* widget) -> bool;
  auto clearFocus() -> void;
  auto focusNext() -> bool;
  auto focusPrevious() -> bool;
  auto focus(Direction direction) -> bool;
  auto handleKeyboardInput(const ncinput& input) -> bool;

private:
  std::vector<Widget*> focusables_;
  Widget* focused_ = nullptr;

  auto setFocusedWidget(Widget* widget) -> void;
  auto findIndex(Widget* widget) const -> std::vector<Widget*>::size_type;
  auto focusByIndex(std::vector<Widget*>::size_type index) -> bool;
  auto bestDirectionalCandidate(Direction direction) const -> Widget*;
};

} // namespace notui