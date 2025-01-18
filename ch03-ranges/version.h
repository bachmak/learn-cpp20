#pragma once

enum class Version {
  Iterator,
  Ranges,
};

template <Version version>
concept VersionIterator = (version == Version::Iterator);

template <Version version>
concept VersionRanges = (version == Version::Ranges);

static_assert(VersionIterator<Version::Iterator>);
static_assert(!VersionIterator<Version::Ranges>);

static_assert(VersionRanges<Version::Ranges>);
static_assert(!VersionRanges<Version::Iterator>);
