#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>
#include <unordered_set>
#include <vector>

/*
 * std::views::all
 *
 * As the name implies, std::views::all creates a view to the range that
 * contains all its elements. Although might seem redundant, it guarantees that
 * the resulting value is a non-owning view to the range. The examples show how
 * it converts the input range to a view only if necessary (leaving view types
 * as is).
 *
 * Conceptually, it can be seen as an "entry point" to the world of views.
 */
namespace all {
constexpr auto example() -> void {
  using std::ranges::ref_view;

  // owning container -> ref_view
  {
    auto range = std::vector{1, 2, 3, 4, 5};
    auto all = range | std::views::all;
    static_assert(std::same_as<decltype(all), ref_view<decltype(range)>>);
  }

  // owning container -> ref_view
  {
    auto range = std::unordered_set{1, 2, 3, 4, 5};
    auto all = range | std::views::all;
    static_assert(std::same_as<decltype(all), ref_view<decltype(range)>>);
  }

  // owning container -> ref_view
  {
    auto range = std::string("example string");
    auto all = range | std::views::all;
    static_assert(std::same_as<decltype(all), ref_view<decltype(range)>>);
  }

  // container view -> original type
  {
    auto range = std::string_view("example string view");
    auto all = range | std::views::all;
    static_assert(std::same_as<decltype(all), decltype(range)>);
  }

  // container view -> original type
  {
    auto owner = std::to_array({1, 2, 3, 4, 5});
    auto range = std::span<int>{owner};
    auto all = range | std::views::all;
    static_assert(std::same_as<decltype(all), decltype(range)>);
  }
}
} // namespace all

/*
 * std::views::counted
 *
 * std::views::counted operates on an iterator, and a size and creates a view to
 * a specified range.
 *
 * Conceptually, it can be seen as a generic std::string_view constructor,
 * accepting iterator instead of char* and count instead of string length.
 */
namespace counted {
template <std::ranges::range Range> struct Input {
  constexpr Input(Range range, int count, int offset)
      : range(std::move(range)), count(count), offset(offset) {}

  constexpr Input(Range range, int count) : Input(std::move(range), count, 0) {}

  Range range;
  int count;
  int offset;
};

constexpr auto test(const auto &input, const std::ranges::range auto &expected)
    -> bool {
  auto first = std::ranges::begin(input.range) + input.offset;
  auto actual = std::views::counted(first, input.count);
  return std::ranges::equal(actual, expected);
}

using vec = std::vector<int>;

static_assert(test(Input(vec({1, 2, 3, 4}), 0), vec()));
static_assert(test(Input(vec({}), 0), vec()));

static_assert(test(Input(vec({1}), 0, 1), vec()));
static_assert(test(Input(vec({1}), 1, 0), vec({1})));

static_assert(test(Input(vec({1, 2, 3, 4}), 1, 3), vec({4})));
static_assert(test(Input(vec({1, 2, 3, 4}), 2, 2), vec({3, 4})));
static_assert(test(Input(vec({1, 2, 3, 4}), 4, 0), vec({1, 2, 3, 4})));
} // namespace counted

/*
 * std::views::common
 *
 * std::views::common has a simple purpose - convert whatever range it receives
 * into a view that is a common range (has begin and end iterator of the same
 * type). The primary use-case is when there's some pre C++-20 code that expects
 * an iterator pair of the same type.
 */
namespace common {

/*
 * Here we emulate a pre C++-20 function that accepts two iterators of the same
 * type to check if the specified range only consists of odd numbers.
 */
namespace legacy_iterator_compatibility_test {
auto is_odd(int x) -> bool { return x % 2 == 1; }

template <typename It> auto legacy_check_all_odd(It first, It last) {
  return std::all_of(first, last, is_odd);
}

auto example() -> void {
  auto numbers = std::vector{1, 2, 3, 4, 5};
  auto odd_numbers = numbers | std::views::filter(is_odd) | std::views::common;

  // only runtime check (there is an issue with std::views::filter that doesn't
  // allow to use it in constexpr context)
  assert(legacy_check_all_odd(std::ranges::begin(odd_numbers),
                              std::ranges::end(odd_numbers)));
}
}; // namespace legacy_iterator_compatibility_test

/*
 * This test-case shows how to use std::views::common to create containers using
 * an iterator pair.
 *
 * Overall, use-cases include:
 * - interfacing with legacy algorithms/APIs
 * - chaining with standard algorithms
 * - working with standard containers (or use C++23 std::ranges::to)
 */
namespace container_conversion_test {
// some converter function
constexpr auto vec_to_string(const std::vector<char> &vec) -> std::string {
  return {vec.begin(), vec.end()};
}

template <std::ranges::range Range> struct Input {
  constexpr Input(Range range, int take)
      : range(std::move(range)), take(take) {}

  Range range;
  int take;
};

constexpr auto test(const auto &input, const std::string &expected) {
  auto common =
      input.range |                  //
      std::views::take(input.take) | // -> emulate some logic by taking
                                     // the first `take` elements
      std::views::common;            // -> convert to a common range

  auto actual = vec_to_string({
      std::ranges::begin(common), //
      std::ranges::end(common)    //
  });
  return std::ranges::equal(actual, expected);
}

static_assert(test(Input(std::vector{'h', 'e', 'l', 'l', 'o'}, 3), "hel"));
static_assert(test(Input(std::vector{'h', 'e', 'l', 'l', 'o'}, 0), ""));
} // namespace container_conversion_test
} // namespace common

/*
 * std::views::drop
 *
 * std::views::drop skips the fist N elements from the input sequence. It is
 * safe to pass a number exceeding the number of elements in the input sequence.
 * The number must be positive though.
 */
namespace drop {

template <std::ranges::range Range> struct Input {
  constexpr Input(Range range, int drop)
      : range(std::move(range)), drop(drop) {}

  Range range;
  int drop;
};

constexpr auto test(const auto &input, const std::ranges::range auto &expected)
    -> bool {
  auto actual = input.range | std::views::drop(input.drop);
  return std::ranges::equal(actual, expected);
}

static_assert(test(Input(std::vector{1, 2, 3}, 1), std::to_array({2, 3})));
static_assert(test(Input(std::vector{1, 2, 3}, 3), std::vector<int>()));
static_assert(test(Input(std::string("abc"), 2), std::string("c")));
// static_assert(test(Input(std::string("abc"), -1), std::string())); -> UB
static_assert(test(Input(std::string_view(), 0), std::string()));
static_assert(test(Input(std::views::iota(1, 4), 2), std::to_array({3})));
static_assert(test(Input(std::views::iota(1, 4), 100), std::vector<int>()));
} // namespace drop

/*
 * std::views::drop_while
 *
 * Similarly to std::views::drop, std::views::drop_while creates a view of
 * elements skipping the elements in the beginning, until the predicate returns
 * false
 */
namespace drop_while {
template <typename Func, typename Range>
concept BoolPredicate =
    std::ranges::range<Range> &&                               //
    std::invocable<Func, std::ranges::range_value_t<Range>> && //
    std::same_as<std::invoke_result_t<Func, std::ranges::range_value_t<Range>>,
                 bool>;

template <std::ranges::range Range, BoolPredicate<Range> Pred> struct Input {
  constexpr Input(Range range, Pred pred)
      : range(std::move(range)), pred(std::move(pred)) {}

  Range range;
  Pred pred;
};

constexpr auto test(const auto &input, const std::ranges::range auto &expected)
    -> bool {
  auto actual = input.range | std::views::drop_while(input.pred);
  return std::ranges::equal(actual, expected);
}

using sv = std::string_view;
using vec = std::vector<int>;

constexpr auto less_than_3 = [](char c) { return c < '3'; };
static_assert(test(Input(sv("12345"), less_than_3), sv("345")));

constexpr auto always_true = [](auto) { return true; };
static_assert(test(Input(vec{1, 2, 3}, always_true), vec{}));

constexpr auto always_false = [](auto) { return false; };
static_assert(test(Input(sv("example"), always_false), sv("example")));

constexpr auto is_space = [](auto c) { return c == ' '; }; // simplified version
static_assert(test(Input(sv("   trim this!"), is_space), sv("trim this!")));

constexpr auto is_negative = [](int n) { return n < 0; };
static_assert(test(Input(vec{-9, -7, -1, 0, 1, 8, 12}, is_negative),
                   vec{0, 1, 8, 12}));
} // namespace drop_while