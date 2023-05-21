#pragma once

namespace vsm {

template<typename KeySelector, typename Hasher, typename Comparator>
struct hash_table_policies
{
	[[no_unique_address]] KeySelector select;
	[[no_unique_address]] Hasher hash;
	[[no_unique_address]] Comparator compare;
};

} // namespace vsm
