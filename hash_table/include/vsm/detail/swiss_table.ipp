#ifndef vsm_swiss_table_ipp
#	define vsm_swiss_table_ipp

#	pragma push_macro("get_h1")
#	define get_h1(hash) static_cast<size_t>((hash) >> 7)

#	pragma push_macro("get_h2")
#	define get_h2(hash) static_cast<ctrl>((hash) & 0x7F)
#else
#	undef vsm_swiss_table_ipp

#	pragma pop_macro("get_h1")
#	pragma pop_macro("get_h2")
#endif
