#pragma once

#include <cstdint>

/*
 * This class emulates a type instances of which should be comparable to each
 * other and to some integral values with == and !=.
 */
class MedicalRecordNumber {
public:
  MedicalRecordNumber() = default;
  constexpr explicit MedicalRecordNumber(uint64_t mrn) : mrn_(mrn) {}

// starting from C++20, these two functions are just enough to cover all
// possible equality and inequality comparisons between MedicalRecordNumber's to
// each other and to uint64's
//
// it works because the compiler is now able to reverse and rewrite functions
#ifdef __cpp_impl_three_way_comparison
  constexpr bool operator==(const MedicalRecordNumber &other) const = default;
  constexpr bool operator==(uint64_t other) const { return other == mrn_; }
#else
  // before C++20 we would have to implement these 6 boilerplate functions
  constexpr bool operator==(MedicalRecordNumber other) const {
    return other.mrn_ == mrn_;
  }

  constexpr bool operator!=(MedicalRecordNumber other) const {
    return !(*this == other);
  }

  constexpr friend bool operator==(MedicalRecordNumber rec, uint64_t num) {
    return rec.mrn_ == num;
  }

  constexpr friend bool operator!=(MedicalRecordNumber rec, uint64_t num) {
    return !(rec == num);
  }

  constexpr friend bool operator==(uint64_t num, MedicalRecordNumber rec) {
    return (rec == num);
  }

  constexpr friend bool operator!=(uint64_t num, MedicalRecordNumber rec) {
    return !(rec == num);
  }
#endif

private:
  uint64_t mrn_;
};

/*
 * Here we test different combinations of expressions with the operators.
 * They all should work with or without C++20, but the implementations are
 * fairly different (see MedicalRecordNumber).
 */
namespace medical_record_number_test {

static_assert(MedicalRecordNumber(1) == MedicalRecordNumber(1));

static_assert(MedicalRecordNumber(1) != MedicalRecordNumber(3));

static_assert(MedicalRecordNumber(1) == 1ul);
static_assert(MedicalRecordNumber(1) != 3ul);

static_assert(1ul == MedicalRecordNumber(1));
static_assert(3ul != MedicalRecordNumber(1));

static_assert(MedicalRecordNumber(1) == 1ul == MedicalRecordNumber(1));
static_assert(MedicalRecordNumber(1) == MedicalRecordNumber(1));
static_assert(1ul == MedicalRecordNumber(1) == 1ul);

} // namespace medical_record_number_test