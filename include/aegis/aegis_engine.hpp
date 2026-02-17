#ifndef AEGIS_ENGINE_HPP
#define AEGIS_ENGINE_HPP

#include <immintrin.h>
#include <stdint.h>

namespace aegis {

/**
 * AEGIS CONTEXT V2 (AES-256 Ready)
 * 15 round keys required for AES-256 (0 through 14)
 */
struct AegisContext {
    __m128i round_keys[15]; 
    int rounds; // Set to 10 for AES-128, 14 for AES-256
};

/**
 * SIGN-BIT GHASH REDUCTION
 * Performs the GF(2^128) reduction without carry-flag branches.
 * Uses the Neptune Sign-Bit Bridge pattern to "smear" the MSB into a mask.
 */
static inline __m128i ghash_reduce(__m128i x) {
    const __m128i poly = _mm_set_epi32(0, 0, 0, 0xE1000000);
    
    // Extract MSB to sign position across lanes
    __m128i top_bit = _mm_srai_epi32(x, 31); 
    // Smear the top bit of the highest 32-bit lane across the whole mask
    __m128i mask = _mm_shuffle_epi32(top_bit, _MM_SHUFFLE(3, 3, 3, 3));
    
    // Branchless XOR reduction: (x << 1) ^ (mask & poly)
    // Note: In a full GCM, bit-reflection/endianness must be handled at the API level.
    return _mm_xor_si128(_mm_slli_epi64(x, 1), _mm_and_si128(mask, poly));
}

/**
 * AEGIS MULTI-WAY KERNEL
 * 4-way Interleaving to hide AES-NI latency (approx 4 cycles per instruction).
 * Supports both 10-round (128-bit) and 14-round (256-bit) modes.
 */
static inline void encrypt_4way(const AegisContext& ctx, __m128i* blocks) {
    // Round 0: Initial AddRoundKey
    __m128i rk0 = ctx.round_keys[0];
    blocks[0] = _mm_xor_si128(blocks[0], rk0);
    blocks[1] = _mm_xor_si128(blocks[1], rk0);
    blocks[2] = _mm_xor_si128(blocks[2], rk0);
    blocks[3] = _mm_xor_si128(blocks[3], rk0);

    // Rounds 1 to (N-1): Interleaved Middle Rounds
    // This loop allows the execution ports to stay saturated while waiting 
    // for the 4-cycle latency of each aesenc instruction to resolve.
    for (int r = 1; r < ctx.rounds; ++r) {
        __m128i rk = ctx.round_keys[r];
        blocks[0] = _mm_aesenc_si128(blocks[0], rk);
        blocks[1] = _mm_aesenc_si128(blocks[1], rk);
        blocks[2] = _mm_aesenc_si128(blocks[2], rk);
        blocks[3] = _mm_aesenc_si128(blocks[3], rk);
    }

    // Final Round: aesenclast
    __m128i rk_last = ctx.round_keys[ctx.rounds];
    blocks[0] = _mm_aesenclast_si128(blocks[0], rk_last);
    blocks[1] = _mm_aesenclast_si128(blocks[1], rk_last);
    blocks[2] = _mm_aesenclast_si128(blocks[2], rk_last);
    blocks[3] = _mm_aesenclast_si128(blocks[3], rk_last);
}

} // namespace aegis
#endif
