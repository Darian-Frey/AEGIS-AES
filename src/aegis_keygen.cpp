#include "../include/aegis/aegis_engine.hpp"

namespace aegis {

/**
 * GHOST KEY EXPANSION (AES-256)
 * Uses SIMD instructions to expand a 256-bit key into 15 round keys.
 * No scalar bit-twiddling, entirely register-resident.
 */
static inline __m128i assist(__m128i temp1, __m128i temp2) {
    __m128i temp3 = _mm_aeskeygenassist_si128(temp1, 0x00);
    temp3 = _mm_shuffle_epi32(temp3, _MM_SHUFFLE(3, 3, 3, 3));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    return _mm_xor_si128(temp2, temp3);
}

void expand_key_256(const uint8_t* key, AegisContext& ctx) {
    ctx.rounds = 14;
    __m128i xmm0 = _mm_loadu_si128((__m128i*)key);
    __m128i xmm1 = _mm_loadu_si128((__m128i*)(key + 16));
    
    ctx.round_keys[0] = xmm0;
    ctx.round_keys[1] = xmm1;

    // Manual unrolling for maximum pipeline saturation
    __m128i temp;
    
    // Round 2-3
    temp = _mm_aeskeygenassist_si128(xmm1, 0x01);
    temp = _mm_shuffle_epi32(temp, 0xff);
    xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
    xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
    xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
    xmm0 = _mm_xor_si128(xmm0, temp);
    ctx.round_keys[2] = xmm0;
    xmm1 = assist(xmm0, xmm1);
    ctx.round_keys[3] = xmm1;

    // Rounds 4-14 follow a similar pattern (simplified for the loop)
    // Note: In a production AEGIS kernel, we unroll this fully.
    uint8_t rcons[] = {0x02, 0x04, 0x08, 0x10, 0x20, 0x40};
    for(int i = 4; i < 14; i += 2) {
        temp = _mm_aeskeygenassist_si128(xmm1, rcons[(i/2)-2]);
        temp = _mm_shuffle_epi32(temp, 0xff);
        xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
        xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
        xmm0 = _mm_xor_si128(xmm0, _mm_slli_si128(xmm0, 4));
        xmm0 = _mm_xor_si128(xmm0, temp);
        ctx.round_keys[i] = xmm0;
        
        if (i + 1 < 15) {
            xmm1 = assist(xmm0, xmm1);
            ctx.round_keys[i+1] = xmm1;
        }
    }
}

} // namespace aegis
