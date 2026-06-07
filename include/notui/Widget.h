#pragma once

#include <notcurses/notcurses.h>
#include <vector>
#include "notui/Layout.h"

namespace notui {

class FocusManager;

class Widget {

public:
  Widget(Widget&&) = delete;
  auto operator=(Widget&&) -> Widget& = delete;
  Widget(struct ncplane* parent, int y, int x, int height, int width);
  virtual ~Widget();

  Widget(const Widget&) = delete;
  auto operator=(const Widget&) -> Widget& = delete;

  virtual auto acceptsFocus() const -> bool { return false; }
  virtual auto onFocus() -> void {}
  virtual auto onBlur() -> void {}

  virtual auto render() -> void = 0;
  virtual auto handleInput(const ncinput& input) -> bool = 0;
  virtual auto resizeAndMove(int y, int x, int height, int width) -> void;
  virtual auto collectFocusable(std::vector<Widget*>& widgets) -> void;
  virtual auto setFocusManager(FocusManager* manager) -> void;

  [[nodiscard]] auto getPlane() const -> struct ncplane* { return plane_; }
  [[nodiscard]] auto getHeight() const -> int { return height_; }
  [[nodiscard]] auto getWidth() const -> int { return width_; }
  [[nodiscard]] auto isFocused() const -> bool { return is_focused_; }
  auto getAbsolutePosition(int& y, int& x) const -> void;
 
  auto setHeightPolicy(SizeMode mode, int value = 0) -> void { height_policy_ = {mode, value}; }
  auto setWidthPolicy(SizeMode mode, int value = 0) -> void { width_policy_ = {mode, value}; }

  [[nodiscard]] auto getHeightPolicy() const -> const LayoutPolicy& { return height_policy_; }
  [[nodiscard]] auto getWidthPolicy() const -> const LayoutPolicy& { return width_policy_; }

protected:
  auto requestFocus() -> void;

  LayoutPolicy height_policy_;
  LayoutPolicy width_policy_;

  struct ncplane* plane_ = nullptr;
  int height_;
  int width_;
  bool is_focused_ = false;
  FocusManager* focus_manager_ = nullptr;

  friend class FocusManager;
};

}