#pragma once

#include "version.h"

#include <string>
#include <utility>

/*
 * We define a concept to allow for string type customization. This is
 * particularly useful in the context of compile-time testing, where usage of
 * std::string has some limitations.
 *
 * This would definitely be an overkill for real usage, but great for practice
 * purposes.
 */
template <typename T>
concept StringConcept = std::ranges::range<T> && //
                        std::same_as<std::ranges::range_value_t<T>, char>;

/*
 * Nothing special here, except the detail that we define a type alias str_type
 * to check the underlying string type when checking a type against Book
 * concept.
 */
template <StringConcept String> struct Book {
  using str_type = String;

  String title;
  String isbn;
};

/*
 * Check the type is Book with whatever underlying string type, which satisfies
 * the StringConcept, of course.
 */
template <typename T>
concept BookConcept = std::same_as<T, Book<typename T::str_type>>;

/*
 * Here we implement an equality operator, but for the BookConcept rather that
 * for Book type with template parameters.
 *
 * Seems like a great way to separate layers of abstractions by not repeating
 * StringConcept in every function that works with Books.
 */
constexpr auto operator==(const BookConcept auto &lhs,
                          const BookConcept auto &rhs) -> bool {
  return std::tie(lhs.title, lhs.isbn) == std::tie(rhs.title, rhs.isbn);
}

/*
 * Here we define a book list concept. Basically, it's introduced to be able to
 * use other containers and not just vector. Again, this is useful in
 * compile-time testing context, to use std::array of books.
 *
 * The cool thing is the range_value_t function which is more generic than the
 * classical typename T::value_type, as it doesn't require a range-like type to
 * have the value_type defined. So, it could be a C-style array, an iterator
 * pair, an STL-container, etc.
 */
template <typename T>
concept BooksConcept = std::ranges::range<T> && //
                       BookConcept<std::ranges::range_value_t<T>>;

/*
 * Sort function implemented with pre C++-20 sort algorithm.
 *
 * The new thing to me here is that functions can be statically dispatched based
 * on different require-clauses, which is cool as this method allows to keep the
 * function parameter list unpolluted by dispatch details. Yet it doesn't
 * require to have a more general overload, like with function specializations.
 */
template <Version version>
  requires VersionIterator<version>
constexpr auto sort(BooksConcept auto &books) -> void {
  // define a comparator, which might be seen as cumbersome
  auto cmp = [](const auto &lhs, const auto &rhs) {
    return lhs.title < rhs.title;
  };

  // begin, end, cumbersome comparator - so much code to type... :)
  std::sort(std::begin(books), std::end(books), cmp);
}

/*
 * Here's the version implemented with new std::ranges facilities.
 *
 * Granted, we've probably made the code overcomplicated with concepts, but the
 * point is that ranges allow to write more laconic code by using projections
 * and passing them to predefined comparators.
 */
template <Version version>
  requires VersionRanges<version>
constexpr auto sort(BooksConcept auto &books) -> void {
  // here we have to deduce the book type (the cost for the generalization)
  using BookType = std::ranges::range_value_t<decltype(books)>;
  // but the sorting part is cool:
  // - pass the range as a whole,
  // - define a comparator as two parts: the algorithm (std::less by default),
  //   and the projection to sort against the specified field
  std::ranges::sort(books, {}, &BookType::title);
}

/*
 * Non-mutating version of sort
 */
template <Version version>
constexpr auto sorted(const BooksConcept auto &books) {
  auto copy = books;
  sort<version>(copy);
  return copy;
}

/*
 * Here our compile-time tests
 */
struct sort_test {
  // helper function to test an implementation of a specific version
  template <Version version>
  consteval auto test(const BooksConcept auto &input,
                      const BooksConcept auto &expected) -> bool {
    auto actual = sorted<version>(input);
    return std::ranges::equal(actual, expected);
  }

  consteval auto operator()() -> void {
    using Book = Book<std::string_view>;

    constexpr auto input = std::to_array<Book>({
        {"Functional programming in C++", "978-3-20-148410-0"},
        {"Effective C++", "978-3-16-148410-0"},
    });
    constexpr auto expected = std::to_array<Book>({
        {"Effective C++", "978-3-16-148410-0"},
        {"Functional programming in C++", "978-3-20-148410-0"},
    });

    static_assert(test<Version::Iterator>(input, expected));
    static_assert(test<Version::Ranges>(input, expected));
  }
};