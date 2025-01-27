#pragma once

/*
 * In the following examples, we see how the compiler automatically deduces the
 * ordering category for aggregate types, based on ordering categories of the
 * members. The important part is that it always uses the common category, which
 * is least strict among the members' ordering categories.
 *
 * The category value can be obtained by the std::common_comparison_category
 * meta-function.
 */

struct Weak {
  auto operator<=>(const Weak &) const -> std::weak_ordering = default;
};

struct Strong {
  auto operator<=>(const Strong &) const -> std::strong_ordering = default;
};

struct Partial {
  auto operator<=>(const Partial &) const -> std::partial_ordering = default;
};

template <typename T1, typename T2> struct Combined {
  T1 first;
  T2 second;

  auto operator<=>(const Combined &) const = default;
};

template <typename T> using OrderingT = std::compare_three_way_result_t<T>;

namespace Test1 {

using Type = Combined<Weak, Strong>;
static_assert(std::same_as<OrderingT<Type>, std::weak_ordering>);

} // namespace Test1

namespace Test2 {

using Type = Combined<Weak, Weak>;
static_assert(std::same_as<OrderingT<Type>, std::weak_ordering>);

} // namespace Test2

namespace Test3 {

using Type = Combined<Strong, Strong>;
static_assert(std::same_as<OrderingT<Type>, std::strong_ordering>);

} // namespace Test3

namespace Test4 {

using Type = Combined<Strong, Partial>;
static_assert(std::same_as<OrderingT<Type>, std::partial_ordering>);

} // namespace Test4