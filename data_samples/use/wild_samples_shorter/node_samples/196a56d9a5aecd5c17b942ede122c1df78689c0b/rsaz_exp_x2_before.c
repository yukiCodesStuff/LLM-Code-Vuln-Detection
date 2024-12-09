#  define ALIGN64
# endif

# define ALIGN_OF(ptr, boundary) \
    ((unsigned char *)(ptr) + (boundary - (((size_t)(ptr)) & (boundary - 1))))

/* Internal radix */
# define BITS2WORD8_SIZE(x)  (((x) + 7) >> 3)
# define BITS2WORD64_SIZE(x) (((x) + 63) >> 6)

static ossl_inline uint64_t get_digit52(const uint8_t *in, int in_len);
static ossl_inline void put_digit52(uint8_t *out, int out_len, uint64_t digit);
static void to_words52(BN_ULONG *out, int out_len, const BN_ULONG *in,
                       int in_bitsize);
    in_str = (uint8_t *)in;

    for (; in_bitsize >= (2 * DIGIT_SIZE); in_bitsize -= (2 * DIGIT_SIZE), out += 2) {
        out[0] = (*(uint64_t *)in_str) & DIGIT_MASK;
        in_str += 6;
        out[1] = ((*(uint64_t *)in_str) >> 4) & DIGIT_MASK;
        in_str += 7;
        out_len -= 2;
    }

        uint8_t *out_str = (uint8_t *)out;

        for (; out_bitsize >= (2 * DIGIT_SIZE); out_bitsize -= (2 * DIGIT_SIZE), in += 2) {
            (*(uint64_t *)out_str) = in[0];
            out_str += 6;
            (*(uint64_t *)out_str) ^= in[1] << 4;
            out_str += 7;
        }

        if (out_bitsize > DIGIT_SIZE) {