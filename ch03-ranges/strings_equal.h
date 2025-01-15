#include <algorithm>
#include <ranges>
#include <string>

enum class Version {
  Iterator,
  Ranges,
};

template <Version v>
constexpr bool strings_equal(const std::string &lhs, const std::string &rhs);

template <>
constexpr bool strings_equal<Version::Iterator>(const std::string &lhs,
                                                const std::string &rhs) {
  // downsides:
  // - code duplication (repetitions of begins, ends, and container names)
  //
  // advantages:
  // - partial application of the algorithm (although quite rare)
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <>
constexpr bool strings_equal<Version::Ranges>(const std::string &lhs,
                                              const std::string &rhs) {
  // drops the need for writing begins and ends repetitively,
  // now it's easy to see that the algorithm is applied to the collections as
  // wholes
  return std::ranges::equal(lhs, rhs);
}

template <Version version> constexpr void strings_equal_test() {
  static_assert(!strings_equal<version>("Hello", "Bello"));
  static_assert(strings_equal<version>("Hello", "Hello"));
}

static_assert((strings_equal_test<Version::Iterator>(), true));
static_assert((strings_equal_test<Version::Ranges>(), true));
