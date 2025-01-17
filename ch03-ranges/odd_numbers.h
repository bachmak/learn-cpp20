#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

#include "version.h"

constexpr bool is_odd(int n) { return n % 2 == 1; }
constexpr int mul_by_2(int n) { return n * 2; }

template <Version>
constexpr std::vector<int> doubled_odd_numbers(const std::vector<int> &numbers);

template <>
constexpr std::vector<int>
doubled_odd_numbers<Version::Iterator>(const std::vector<int> &numbers)
{
  auto oddNumbers = std::vector<int>();

  std::copy_if(begin(numbers), end(numbers), std::back_inserter(oddNumbers),
               is_odd);

  auto results = std::vector<int>();

  std::transform(begin(oddNumbers), end(oddNumbers),
                 std::back_inserter(results), mul_by_2);

  return results;
}

template <>
constexpr std::vector<int>
doubled_odd_numbers<Version::Ranges>(const std::vector<int> &numbers)
{
  return numbers |                         //
         std::views::filter(is_odd) |      //
         std::views::transform(mul_by_2) | //
         std::ranges::to<std::vector>();
}

template <Version version>
consteval void doubled_odd_numbers_test()
{
  auto test = [](std::vector<int> input, std::vector<int> expected)
  {
    auto actual = doubled_odd_numbers<version>(input);
    return std::ranges::equal(expected, actual);
  };

  static_assert(test({2, 3, 4, 5, 6}, {6, 10}));
  static_assert(test({}, {}));
  static_assert(test({0, 0}, {}));
  static_assert(test({1, 9}, {2, 18}));
}

template <Version ... version>
consteval bool doubled_odd_numbers_test_all() {
  ( doubled_odd_numbers_test<version>(), ... );
  return true;
}

static_assert((doubled_odd_numbers_test_all<Version::Iterator, Version::Ranges>(), true));
