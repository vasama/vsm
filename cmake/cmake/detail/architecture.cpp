#  if defined(__i386__) \
   || defined(__i486__) \
   || defined(__i586__) \
   || defined(__i686__) \
   || defined(_M_IX86)
set(architecture x86)
#elif defined(__x86_64) \
   || defined(__x86_64__) \
   || defined(__amd64) \
   || defined(__amd64__) \
   || defined(_M_X64)
set(architecture x86_64)
#else
message(FATAL_ERROR "Unrecognized system processor")
#endif
