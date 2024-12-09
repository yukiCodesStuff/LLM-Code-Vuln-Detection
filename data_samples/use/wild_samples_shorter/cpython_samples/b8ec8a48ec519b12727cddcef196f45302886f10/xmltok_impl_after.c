                       const char *end,
                       POSITION *pos)
{
  while (ptr < end) {
    switch (BYTE_TYPE(enc, ptr)) {
#define LEAD_CASE(n) \
    case BT_LEAD ## n: \
      ptr += n; \