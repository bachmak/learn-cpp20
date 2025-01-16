#pragma once

#include <iterator>
#include <ranges>

#include "version.h"

// not implemented general version of Use
template <Version version> struct Use {
  constexpr auto operator()(auto &c) -> void { static_assert(sizeof(c) == -1); }
};

template <> struct Use<Version::Iterator> {
  constexpr auto operator()(const auto &c) {
    // here we use a trick to lookup both std::begin and free begin function,
    // possibly implemented for the type
    //
    // downsides:
    // - not intuitive or obvious code;
    // - we lack useful functionality of std::begin when using a free begin
    // function (in contrast with the case where we implement the begin method)
    using std::begin;
    return begin(c);
  }
};

template <> struct Use<Version::Ranges> {
  constexpr auto operator()(const auto &c) {
    // works for both member and free functions
    return std::ranges::begin(c);
  }
};

struct Container {
  constexpr static auto value = 42;
};

constexpr const int *begin(const Container) { return &Container::value; }

struct OtherContainer {
  constexpr static auto value = 43;

  constexpr const int *begin() const { return &value; }
};

template <Version version> constexpr void use_test() {
  auto use = Use<version>();
  static_assert(*use(Container()) == Container::value);
  static_assert(*use(OtherContainer()) == OtherContainer::value);
}

static_assert((use_test<Version::Iterator>(), true));
static_assert((use_test<Version::Ranges>(), true));
