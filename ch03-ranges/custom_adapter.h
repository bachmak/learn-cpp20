#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>
#include <string>
#include <vector>

/*
 * Our custom range adapter. We just wrap an existing adapter (or a chain of
 * multiple such adapters) and pass some additional data to it.
 *
 * Benefits: less code duplication, clear naming
 *
 * Note: capture by value, as the lambda is invoked later, as the adapter
 * actually gets called.
 */
constexpr auto add_currency(const std::string &suffix) {
  return std::views::transform(
      [suffix](auto s) { return std::to_string(s) + suffix; });
}

inline void test() {
  auto prices = std::vector{3.95, 6.0, 95.4, 10.95, 12.90, 5.50};
  auto expected = {"3.95USD", "6.0USD", "5.50USD"};

  auto actual = prices |
                std::views::filter([](auto price) { return price < 10.0; }) |
                add_currency("USD");

  assert(std::ranges::equal(actual, expected));
}