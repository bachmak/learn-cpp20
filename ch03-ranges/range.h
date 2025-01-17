#pragma once

#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>

/*
 * What's range?
 *
 * It's something that has begin and end functions defined.
 *
 * The following type is a classical iterator pair, since its begin and end
 * methods have the same return type. A common range, in other words.
 */
template <typename T> struct RangeV1 {
  T begin();
  T end();
};

/*
 * The following type defines an infinite range, because its end method returns
 * a value of special type — sentinel.
 */
template <typename T> struct RangeV2 {
  T begin();
  std::default_sentinel_t end();
};

/*
 * The example function shows how to convert a sentinel range to one that can be
 * consumed by classical iterator based algorithms.
 */
template <int limit>
constexpr auto sum_while_greater(const std::vector<int> &numbers) -> int {
  auto pred = [](int x) { return x > limit; };
  auto rng = numbers |
             // take elements from the start until there's one that doesn't
             // satisfy the predicate
             std::views::take_while(pred) |
             // transform the sentinel range returned by take_while to a common
             // range compatible with std::accumulate
             std::views::common;

  return std::accumulate(std::begin(rng), std::end(rng), 0);
}

struct sum_while_greater_test {
  template <int limit>
  consteval auto test(const std::vector<int> &input, int expected) -> bool {
    return sum_while_greater<limit>(input) == expected;
  }

  consteval auto operator()() -> void {
    static_assert(test<5>({1, 2, 3, 4, 5}, 0));
    static_assert(test<0>({1, 2, 3, 4, 5}, 15));
    static_assert(test<4>({5, 4, 3, 2, 1}, 5));
  }
};
