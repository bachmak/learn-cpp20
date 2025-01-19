#pragma once

#include <forward_list>
#include <ranges>
#include <string>
#include <vector>

/*
 * Here we create a little test framework to test what types satisfy what ranges
 * concepts.
 */

enum class range { True, False };
template <typename T> constexpr auto check_is_range() {
  return std::ranges::range<T> ? range::True : range::False;
}

enum class borrowed_range { True, False };
template <typename T> constexpr auto check_is_borrowed_range() {
  return std::ranges::borrowed_range<T> ? borrowed_range::True
                                        : borrowed_range::False;
}

enum class sized_range { True, False };
template <typename T> constexpr auto check_is_sized_range() {
  return std::ranges::sized_range<T> ? sized_range::True : sized_range::False;
}

enum class view { True, False };
template <typename T> constexpr auto check_is_view() {
  return std::ranges::view<T> ? view::True : view::False;
}

enum class input_range { True, False };
template <typename T> constexpr auto check_is_input_range() {
  return std::ranges::input_range<T> ? input_range::True : input_range::False;
}

enum class output_range { True, False };
template <typename T, typename Elem> constexpr auto check_is_output_range() {
  return std::ranges::output_range<T, Elem> ? output_range::True
                                            : output_range::False;
}

enum class forward_range { True, False };
template <typename T> constexpr auto check_is_forward_range() {
  return std::ranges::forward_range<T> ? forward_range::True
                                       : forward_range::False;
}

enum class bidirectional_range { True, False };
template <typename T> constexpr auto check_is_bidirectional_range() {
  return std::ranges::bidirectional_range<T> ? bidirectional_range::True
                                             : bidirectional_range::False;
}

enum class random_access_range { True, False };
template <typename T> constexpr auto check_is_random_access_range() {
  return std::ranges::random_access_range<T> ? random_access_range::True
                                             : random_access_range::False;
}

enum class contiguous_range { True, False };
template <typename T> constexpr auto check_is_contiguous_range() {
  return std::ranges::contiguous_range<T> ? contiguous_range::True
                                          : contiguous_range::False;
}

enum class common_range { True, False };
template <typename T> constexpr auto check_is_common_range() {
  return std::ranges::common_range<T> ? common_range::True
                                      : common_range::False;
}

enum class viewable_range { True, False };
template <typename T> constexpr auto check_is_viewable_range() {
  return std::ranges::viewable_range<T> ? viewable_range::True
                                        : viewable_range::False;
}

template <typename Range, typename Elem, range is_range,
          borrowed_range is_borrowed_range, sized_range is_sized_range,
          view is_view, input_range is_input_range,
          output_range is_output_range, forward_range is_forward_range,
          bidirectional_range is_bidirectional_range,
          random_access_range is_random_access_range,
          contiguous_range is_contiguous_range, common_range is_common_range,
          viewable_range is_viewable_range>
struct Test {
  static_assert(check_is_range<Range>() == is_range);
  static_assert(check_is_borrowed_range<Range>() == is_borrowed_range);
  static_assert(check_is_sized_range<Range>() == is_sized_range);
  static_assert(check_is_view<Range>() == is_view);
  static_assert(check_is_input_range<Range>() == is_input_range);
  static_assert(check_is_output_range<Range, Elem>() == is_output_range);
  static_assert(check_is_forward_range<Range>() == is_forward_range);
  static_assert(check_is_bidirectional_range<Range>() ==
                is_bidirectional_range);
  static_assert(check_is_random_access_range<Range>() ==
                is_random_access_range);
  static_assert(check_is_contiguous_range<Range>() == is_contiguous_range);
  static_assert(check_is_common_range<Range>() == is_common_range);
  static_assert(check_is_common_range<Range>() == is_common_range);
  static_assert(check_is_viewable_range<Range>() == is_viewable_range);
};

constexpr auto test_vec = Test<std::vector<int>, int,
                               range::True,               //
                               borrowed_range::False,     //
                               sized_range::True,         //
                               view::False,               //
                               input_range::True,         //
                               output_range::True,        //
                               forward_range::True,       //
                               bidirectional_range::True, //
                               random_access_range::True, //
                               contiguous_range::True,    //
                               common_range::True,        //
                               viewable_range::True       //
                               >();

constexpr auto test_fwd_list = Test<std::forward_list<int>, std::string,
                                    range::True,                //
                                    borrowed_range::False,      //
                                    sized_range::False,         //
                                    view::False,                //
                                    input_range::True,          //
                                    output_range::False,        //
                                    forward_range::True,        //
                                    bidirectional_range::False, //
                                    random_access_range::False, //
                                    contiguous_range::False,    //
                                    common_range::True,         //
                                    viewable_range::True        //
                                    >();

template <typename T> struct IstreamRange {
  auto begin() -> std::istream_iterator<T>;
  auto end() -> std::istream_iterator<T>;
};

constexpr auto test_istream = Test<IstreamRange<int>, void,    //
                                   range::True,                //
                                   borrowed_range::False,      //
                                   sized_range::False,         //
                                   view::False,                //
                                   input_range::True,          //
                                   output_range::False,        //
                                   forward_range::False,       //
                                   bidirectional_range::False, //
                                   random_access_range::False, //
                                   contiguous_range::False,    //
                                   common_range::True,         //
                                   viewable_range::True        //
                                   >();

constexpr auto test_string_view = Test<std::string_view, char,    //
                                       range::True,               //
                                       borrowed_range::True,      //
                                       sized_range::True,         //
                                       view::True,                //
                                       input_range::True,         //
                                       output_range::False,       //
                                       forward_range::True,       //
                                       bidirectional_range::True, //
                                       random_access_range::True, //
                                       contiguous_range::True,    //
                                       common_range::True,        //
                                       viewable_range::True       //
                                       >();

constexpr auto test_const_vec = Test<const std::vector<int>,    //
                                     int,                       //
                                     range::True,               //
                                     borrowed_range::False,     //
                                     sized_range::True,         //
                                     view::False,               //
                                     input_range::True,         //
                                     output_range::False,       //
                                     forward_range::True,       //
                                     bidirectional_range::True, //
                                     random_access_range::True, //
                                     contiguous_range::True,    //
                                     common_range::True,        //
                                     viewable_range::False      // NOTE
                                     >();