#include "notui/FocusManager.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <optional>

namespace notui {

auto FocusManager::rebuild(Widget& root) -> void {
  Widget* previous = focused_;

  focusables_.clear();
  root.collectFocusable(focusables_);

  if (focusables_.empty()) {
    setFocusedWidget(nullptr);
    return;
  }

  if (previous != nullptr) {
    const auto existing = std::find(focusables_.begin(), focusables_.end(), previous);
    if (existing != focusables_.end()) {
      setFocusedWidget(*existing);
      return;
    }
  }

  setFocusedWidget(focusables_.front());
}

auto FocusManager::focusedWidget() const -> Widget* {
  return focused_;
}

auto FocusManager::focusWidget(Widget* widget) -> bool {
  if (widget == nullptr) {
    clearFocus();
    return true;
  }

  const auto it = std::find(focusables_.begin(), focusables_.end(), widget);
  if (it == focusables_.end()) {
    return false;
  }

  setFocusedWidget(widget);
  return true;
}

auto FocusManager::clearFocus() -> void {
  setFocusedWidget(nullptr);
}

auto FocusManager::focusNext() -> bool {
  if (focusables_.empty()) {
    return false;
  }

  if (focused_ == nullptr) {
    return focusByIndex(0);
  }

  const auto current_index = findIndex(focused_);
  const auto next_index = (current_index + 1) % focusables_.size();
  return focusByIndex(next_index);
}

auto FocusManager::focusPrevious() -> bool {
  if (focusables_.empty()) {
    return false;
  }

  if (focused_ == nullptr) {
    return focusByIndex(focusables_.size() - 1);
  }

  const auto current_index = findIndex(focused_);
  const auto previous_index = current_index == 0 ? focusables_.size() - 1 : current_index - 1;
  return focusByIndex(previous_index);
}

auto FocusManager::focus(Direction direction) -> bool {
  Widget* candidate = bestDirectionalCandidate(direction);
  if (candidate == nullptr) {
    return false;
  }

  setFocusedWidget(candidate);
  return true;
}

auto FocusManager::handleKeyboardInput(const ncinput& input) -> bool {
  if (input.id == NCKEY_TAB) {
    return ncinput_shift_p(&input) ? focusPrevious() : focusNext();
  }

  if (input.id == NCKEY_UP) {
    return focus(Direction::Up);
  }

  if (input.id == NCKEY_DOWN) {
    return focus(Direction::Down);
  }

  if (input.id == NCKEY_LEFT) {
    return focus(Direction::Left);
  }

  if (input.id == NCKEY_RIGHT) {
    return focus(Direction::Right);
  }

  return false;
}

auto FocusManager::setFocusedWidget(Widget* widget) -> void {
  if (focused_ == widget) {
    return;
  }

  if (focused_ != nullptr) {
    focused_->is_focused_ = false;
    focused_->onBlur();
  }

  focused_ = widget;

  if (focused_ != nullptr) {
    focused_->is_focused_ = true;
    focused_->onFocus();
  }
}

auto FocusManager::findIndex(Widget* widget) const -> std::vector<Widget*>::size_type {
  const auto it = std::find(focusables_.begin(), focusables_.end(), widget);
  return static_cast<std::vector<Widget*>::size_type>(std::distance(focusables_.begin(), it));
}

auto FocusManager::focusByIndex(std::vector<Widget*>::size_type index) -> bool {
  if (index >= focusables_.size()) {
    return false;
  }

  setFocusedWidget(focusables_[index]);
  return true;
}

auto FocusManager::bestDirectionalCandidate(Direction direction) const -> Widget* {
  if (focusables_.empty()) {
    return nullptr;
  }

  if (focused_ == nullptr) {
    return focusables_.front();
  }

  int current_y = 0;
  int current_x = 0;
  focused_->getAbsolutePosition(current_y, current_x);
  const long long current_center_y = static_cast<long long>(current_y) + focused_->getHeight() / 2;
  const long long current_center_x = static_cast<long long>(current_x) + focused_->getWidth() / 2;

  struct CandidateScore {
    long long distance = std::numeric_limits<long long>::max();
    long long primary_delta = std::numeric_limits<long long>::max();
    long long secondary_delta = std::numeric_limits<long long>::max();
    std::vector<Widget*>::size_type index = 0;
    Widget* widget = nullptr;
  };

  std::optional<CandidateScore> best;

  for (std::vector<Widget*>::size_type index = 0; index < focusables_.size(); ++index) {
    Widget* candidate = focusables_[index];
    if (candidate == nullptr || candidate == focused_ || candidate->getHeight() <= 0 || candidate->getWidth() <= 0) {
      continue;
    }

    int candidate_y = 0;
    int candidate_x = 0;
    candidate->getAbsolutePosition(candidate_y, candidate_x);
    const long long candidate_center_y = static_cast<long long>(candidate_y) + candidate->getHeight() / 2;
    const long long candidate_center_x = static_cast<long long>(candidate_x) + candidate->getWidth() / 2;

    bool direction_matches = false;
    switch (direction) {
      case Direction::Up:
        direction_matches = candidate_center_y < current_center_y;
        break;
      case Direction::Down:
        direction_matches = candidate_center_y > current_center_y;
        break;
      case Direction::Left:
        direction_matches = candidate_center_x < current_center_x;
        break;
      case Direction::Right:
        direction_matches = candidate_center_x > current_center_x;
        break;
    }

    if (!direction_matches) {
      continue;
    }

    const long long dy = candidate_center_y - current_center_y;
    const long long dx = candidate_center_x - current_center_x;
    const long long distance = dy * dy + dx * dx;
    const long long primary_delta = (direction == Direction::Up || direction == Direction::Down)
        ? std::llabs(dy)
        : std::llabs(dx);
    const long long secondary_delta = (direction == Direction::Up || direction == Direction::Down)
        ? std::llabs(dx)
        : std::llabs(dy);

    CandidateScore score{distance, primary_delta, secondary_delta, index, candidate};

    if (!best.has_value() || score.distance < best->distance ||
        (score.distance == best->distance && (score.primary_delta < best->primary_delta ||
        (score.primary_delta == best->primary_delta && (score.secondary_delta < best->secondary_delta ||
        (score.secondary_delta == best->secondary_delta && score.index < best->index)))))) {
      best = score;
    }
  }

  return best.has_value() ? best->widget : nullptr;
}

} // namespace notui