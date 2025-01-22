#pragma once

#include "version.h"

#include <algorithm>
#include <cassert>

/*
 * Here we compare two ways how to add ordering comparison support for a custom
 * type in C++17 and C++20.
 */

/*
 * Common part to reduce code duplication in the String versions for C++17 and
 * C++20.
 */
class StringImpl {
  const char *str_;
  const std::size_t size_;

public:
  template <std::size_t N>
  constexpr explicit StringImpl(const char (&str)[N]) : str_(str), size_(N) {}

  [[nodiscard]] constexpr const char *begin() const { return str_; }
  [[nodiscard]] constexpr const char *end() const { return str_ + size_; }
};

template <Version version> class String;

template <> class String<Version::Cpp17> : public StringImpl {
public:
  using StringImpl::StringImpl;

public:
  /*
   * Before C++20, we had to implement the comparison in 6 operators: ==, !=, <,
   * >, <=, >=, which is error-prone and doesn't contain any real logic.
   */
  constexpr friend bool operator==(const String &lhs, const String &rhs) {
    // optimization: early return to avoid checking full strings with compare
    if (std::distance(lhs.begin(), lhs.end()) !=
        std::distance(rhs.begin(), rhs.end())) {
      return false;
    }

    return compare(lhs, rhs) == Ordering::Equal;
  }

  constexpr friend bool operator!=(const String &lhs, const String &rhs) {
    return !(lhs == rhs);
  }

  constexpr friend bool operator<(const String &lhs, const String &rhs) {
    return compare(lhs, rhs) == Ordering::Less;
  }

  constexpr friend bool operator>(const String &lhs, const String &rhs) {
    return compare(lhs, rhs) == Ordering::Greater;
  }

  constexpr friend bool operator<=(const String &lhs, const String &rhs) {
    return !(lhs > rhs);
  }

  constexpr friend bool operator>=(const String &lhs, const String &rhs) {
    return !(lhs < rhs);
  }

private:
  enum class Ordering { Equal, Less, Greater };

  /*
   * Here we define the actual logic of comparison.
   */
  [[nodiscard]] constexpr static Ordering compare(const String &lhs,
                                                  const String &rhs) {
    // granted, we don't have ranges in C++17, but that's not the point of this
    // code (in C++17, it would be std::equal and std::lexicographical_compare)
    if (std::ranges::equal(lhs, rhs)) {
      return Ordering::Equal;
    }

    if (std::ranges::lexicographical_compare(lhs, rhs)) {
      return Ordering::Less;
    }

    return Ordering::Greater;
  }
};

template <> class String<Version::Cpp20> : public StringImpl {
public:
  using StringImpl::StringImpl;

public:
  /*
   * We could simply define the operator as default, but that would do shallow
   * member-wise comparison, including comparing the values of the pointers,
   * which is not what we want. That's why we implement it manually.
   *
   * Still, that's the only operator we need to support all the 6 operations.
   */
  constexpr auto operator<=>(const String &rhs) const {
    using It = decltype(begin());
    constexpr auto cmp = std::lexicographical_compare_three_way<It, It>;

    return cmp(begin(), end(), rhs.begin(), rhs.end());
  }

  /*
   * Again, we could simply omit this explicit overload, and it would still
   * work. And this would be the case for many types, where comparison is done
   * member-wise. But since our case is special, and we work with pointers, we
   * want to avoid calling std::lexicographical_compare_three_way just to check
   * if the two values are equal.
   *
   */
  constexpr bool operator==(const String &other) const {
    return std::ranges::equal(*this, other);
  }
};

namespace string_test {

template <Version version> struct Test {
  constexpr auto operator()() const {
    using Str = String<version>;

    static_assert(Str("abc") == Str("abc"));
    static_assert(Str("abc") != Str("bbc"));

    static_assert(Str("abc") < Str("bbc"));
    static_assert(Str("abc") <= Str("bbc"));
    static_assert(Str("abc") <= Str("abc"));

    static_assert(Str("bbc") > Str("abca"));
    static_assert(Str("bbc") >= Str("abca"));
    static_assert(Str("bbc") >= Str("bbc"));

    // we return bool to force compiler to run through the static_assert's above
    // when calling the operator from static_assert's with concrete types
    return true;
  }
};

static_assert(Test<Version::Cpp17>{}());
static_assert(Test<Version::Cpp20>{}());

} // namespace string_test