#pragma once

namespace notui {

enum class SizeMode {
    Fixed,      // Exact number of terminal cells (e.g., 3 rows high)
    Percent,    // Percentage of the parent's total size (0 to 100)
    Expand      // Dynamically fill any remaining space left in the container
};

struct LayoutPolicy {
    SizeMode mode = SizeMode::Expand;
    int value = 0; // The unit value (e.g., 5 for Fixed, 30 for Percent)
};

} // namespace notui