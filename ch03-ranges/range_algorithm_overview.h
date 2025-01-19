#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>
#include <unordered_set>
#include <vector>

template <typename Func, typename Range>
concept BoolPredicate =
    std::ranges::range<Range> &&                               //
    std::invocable<Func, std::ranges::range_value_t<Range>> && //
    std::same_as<std::invoke_result_t<Func, std::ranges::range_value_t<Range>>,
                 bool>;

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

/*
 * std::views::elements
 *
 * std::views::elements creates a view that returns N-th element of a tuple-like
 * element in a sequence. The N value is a compile-time constant, like that
 * passed to std::get<N>.
 *
 * Tuple-like types are std::tuple, std::pair, std::array
 * (where N defines an index), or user-defined types that satisfy std::tuple
 * interface: specializations of get, tuple_size, and tuple_element should be
 * provided. The latter case doesn't seem really useful so far, except for some
 * generic template code.
 *
 * std::views::elements is a generalization of std::views::keys and
 * std::views::values
 */
namespace elements {
template <std::ranges::range Range, std::size_t index> struct InputImpl {
  constexpr explicit InputImpl(Range range) : range(std::move(range)) {}

  Range range;
};

// could be nice to achieve the same effect with a deduction guide, but I
// couldn't get it compiling :)
template <std::size_t index, std::ranges::range Range>
constexpr auto Input(Range range) {
  return InputImpl<Range, index>(std::move(range));
}

template <std::ranges::range Range, std::size_t index>
constexpr auto test(const InputImpl<Range, index> &input,
                    const std::ranges::range auto &expected) {
  auto actual = input.range | std::views::elements<index>;
  return std::ranges::equal(actual, expected);
}

namespace Test1 {
using elem_t = std::tuple<int, int>;
constexpr auto array = std::to_array<elem_t>({{1, 0}, {2, 0}, {3, 0}});
static_assert(test(Input<0>(array), std::to_array({1, 2, 3})));
static_assert(test(Input<1>(array), std::to_array({0, 0, 0})));
} // namespace Test1

namespace Test2 {
using elem_t = std::pair<std::string_view, int>;
constexpr auto array = std::to_array<elem_t>({{"hi", 1}, {"b2", 4}, {"no", 2}});
static_assert(test(Input<0>(array), std::to_array({"hi", "b2", "no"})));
static_assert(test(Input<1>(array), std::to_array({1, 4, 2})));
// static_assert(test(Input<2>(array), std::to_array({1, 4, 2}))); -> error
} // namespace Test2

namespace Test3 {
using elem_t = std::array<int, 3>;
constexpr auto array = std::to_array<elem_t>({{1, 2, 3}, {4, 5, 6}});
static_assert(test(Input<0>(array), std::to_array({1, 4})));
static_assert(test(Input<1>(array), std::to_array({2, 5})));
static_assert(test(Input<2>(array), std::to_array({3, 6})));
// static_assert(test(Input<3>(array), std::to_array({3, 6}))); -> error
} // namespace Test3

// other examples could be associative containers and custom tuple-like types
} // namespace elements

/*
 * std::views::filter
 *
 * creates a view with elements satisfying a given predicate. It's pretty much
 * straightforward and does exactly what the name suggests.
 */
namespace filter {

template <std::ranges::range Range, BoolPredicate<Range> Pred> struct Input {
  constexpr Input(Range range, Pred pred)
      : range(std::move(range)), pred(std::move(pred)) {}

  Range range;
  Pred pred;
};

constexpr auto test(const auto &input,
                    const std::ranges::range auto &expected) {
  auto actual = input.range | std::views::filter(input.pred);
  return std::ranges::equal(actual, expected);
}

namespace Test1 {
constexpr auto array = std::to_array({-3, -2, -1, 0, 1, 2, 3});
constexpr auto is_negative = [](auto c) { return c < 0; };
constexpr auto is_zero = [](auto c) { return c == 0; };
constexpr auto is_positive = [](auto c) { return c > 0; };

static_assert(test(Input(array, is_negative), std::to_array({-3, -2, -1})));
static_assert(test(Input(array, is_zero), std::to_array({0})));
static_assert(test(Input(array, is_positive), std::to_array({1, 2, 3})));
} // namespace Test1

namespace Test2 {
using elem_t = std::optional<std::string_view>;
constexpr auto array = std::to_array<elem_t>(
    {"John", "Felix", std::nullopt, "Carl", std::nullopt});

static_assert(test(Input(array, &elem_t::has_value),
                   std::to_array({"John", "Felix", "Carl"})));
} // namespace Test2
} // namespace filter

/*
 * std::vies::join
 *
 * std::views::join 'flattens' a range of ranges into a flat sequence. That's
 * it.
 */
namespace join {
constexpr auto test(const std::ranges::range auto &input,
                    const std::ranges::range auto &expected) -> bool {
  auto actual = input | std::views::join;
  return std::ranges::equal(actual, expected);
}

namespace Test1 {
static_assert(test(std::to_array<std::string_view>({"Hel", "lo", ", world!"}),
                   std::string("Hello, world!")));

using elem_t = std::array<int, 2>;
static_assert(test(std::to_array<elem_t>({{1, 2}, {3, 4}, {5, 6}}),
                   std::to_array({1, 2, 3, 4, 5, 6})));

// could work with C++26, once std::optional gains begin and end
// static_assert(test(std::optional<std::optional<int>>(2),
// std::optional<int>(2)));
} // namespace Test1
} // namespace join

/*
 * std::views::ref
 *
 * This section is intentionally empty as std::ranges::ref_view is more of a
 * low-level range adapter, used to provide a non-owning view to a range. For
 * standard ranges, std::views::all does just the right thing, choosing the
 * right adapter. Unless dealing with custom range types (which might be the
 * case though!), std::views::ref doesn't seem worth using explicitly.
 */
namespace ref {}

/*
 * std::views::reverse
 *
 * Creates a view in which the elements go in the reverse order. Quite simple.
 */
namespace reverse {

constexpr auto test(const std::ranges::range auto &input,
                    const std::ranges::range auto &expected) -> bool {
  auto actual = input | std::views::reverse;
  return std::ranges::equal(actual, expected);
}

static_assert(test(std::string("hello"), std::string("olleh")));
static_assert(test(std::vector{1, 2, 3, 4, 5}, std::vector{5, 4, 3, 2, 1}));

using elem_t = std::array<int, 2>;
static_assert(test(std::to_array<elem_t>({{1, 2}, {3, 4}, {5, 6}}),
                   std::to_array<elem_t>({{5, 6}, {3, 4}, {1, 2}})));
} // namespace reverse

/*
 * std::views::split
 *
 * Splits the given range into a sequence of sub-ranges based on separator.
 * Notes: the separator can be either of the element type (char in an
 * std::string) or of the subrange type (std::string_view in an
 * std::string_view)
 */
namespace split {

template <std::ranges::range Range, typename Separator> struct Input {
  constexpr Input(Range range, Separator separator)
      : range(std::move(range)), separator(std::move(separator)) {}

  Range range;
  Separator separator;
};

constexpr auto test(const auto &input, const std::ranges::range auto &expected)
    -> bool {
  // the std::ranges::to part works only for owning ranges
  auto actual = input.range | std::views::split(input.separator) |
                std::views::join | std::ranges::to<decltype(input.range)>();
  return std::ranges::equal(actual, expected);
}

static_assert(test(Input(std::string("h e l l o"), ' '), std::string("hello")));

} // namespace split