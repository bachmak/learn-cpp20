#pragma once

#include <iostream>
#include <ranges>
#include <vector>

/*
 * Here we're reimplementing std::ranges::take_view to get familiar with
 * the peculiarities of how std::ranges work under the hood.
 */

// restrict ranges to view concept, as we want to be able to copy and move the
// range in a constant time.
//
// derive from the view_interface using CRTP
template <std::ranges::view R>
class custom_take_view
    : public std::ranges::view_interface<custom_take_view<R>> {
  // special private fields: base_, count_
  R base_;
  std::ranges::range_difference_t<R> count_;

public:
  // constructors:
  // - default constructor
  // - constructor from the range, and the count of the range difference type
  custom_take_view() = default;
  constexpr custom_take_view(R base, std::ranges::range_difference_t<R> count)
      : base_(std::move(base)), count_(count) {}

  // special methods for the interface: base, returns the underlying range
  // overloaded for l- and r-values
  constexpr R base() const & { return base_; }
  constexpr R base() && { return std::move(base_); }

  // begin and end methods implement the actual logic
  constexpr auto begin() { return std::ranges::begin(base_); }
  constexpr auto end() {
    return std::ranges::next(std::ranges::begin(base_), count_);
  }
};

// deduction guide to guarantee we always work with views
template <std::ranges::range R>
custom_take_view(R &&, std::ranges::range_difference_t<R>)
    -> custom_take_view<std::views::all_t<R>>;

namespace details {
// as an implementation detail, to allow for the syntax like
// auto view = std::ranges::take_view(3);
// to pass the actual range to it later, using the pipe syntax, for example
//
// restrict the count type to be integral
template <std::integral T> struct custom_take_range_adaptor_closure {
  // we only store the count
  T count;

  // the only constructor
  constexpr custom_take_range_adaptor_closure(T count) : count(count) {}

  // call operator actually creates the take_view, given the range, by passing
  // the stored count value to it, along with the range
  //
  // restrict the range type to be viewable (see the deduction guide for
  // custom_take_view)
  template <std::ranges::viewable_range R>
  constexpr auto operator()(R &&r) const {
    return custom_take_view(std::forward<R>(r), count);
  }
};

// custom_take_range_adaptor does static dispatch, choosing the right type:
// if given the count only, it returns the closure instance
// if given both the count and the range, it returns custom_take_view itself
struct custom_take_range_adaptor {
  template <typename... Args> constexpr auto operator()(Args &&...args) {
    if constexpr (sizeof...(args) == 1) {
      return custom_take_range_adaptor_closure(args...);
    } else {
      return custom_take_view(std::forward<Args>(args)...);
    }
  }
};

// overload the | operator, but for the closure type only
template <std::ranges::viewable_range R, std::integral T>
constexpr auto operator|(R &&r, custom_take_range_adaptor_closure<T> a) {
  return a(std::forward<R>(r));
}
} // namespace details

// for consistency, declare custom_take in views namespace
// the adaptor will choose which type to use, based on the provided arguments
namespace views {
inline details::custom_take_range_adaptor custom_take;
}

void custom_take_view_test() {
  auto is_odd = [](int n) { return n % 2 == 1; };
  const auto n = std::vector{2, 3, 5, 6, 7, 8, 9};
  auto v = n | std::views::filter(is_odd) | views::custom_take(2);
  std::ranges::copy(v, std::ostream_iterator<int>(std::cout, " "));
}