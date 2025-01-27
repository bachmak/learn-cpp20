#pragma once

#include <cstdint>
#include <string>

struct Address {
  std::string city;
  std::string street;
  std::uint32_t street_no;

  /*
   * Here we define our custom comparison operator because we need to change the
   * default member-wise comparison order, i.e. sort by street numbers in
   * descending order.
   */
  auto operator<=>(const Address &other) const {
    // ascending order for cities
    if (auto cmp = city <=> other.city; std::is_neq(cmp)) {
      return cmp;
    }

    // ascending order for street names
    if (auto cmp = street <=> other.street; std::is_neq(cmp)) {
      return cmp;
    }

    // descending (custom) order for street numbers
    if (auto cmp = other.street_no <=> street_no; std::is_neq(cmp)) {
      return cmp;
    }

    return std::strong_ordering::equal;
  }

  // it's fine to have the == operator defaulted
  bool operator==(const Address &) const = default;
};
