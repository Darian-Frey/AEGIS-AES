#ifndef AEGIS_ENGINE_HPP
#define AEGIS_ENGINE_HPP

#include <immintrin.h>
#include <stdint.h>

namespace aegis {

/**
 * AEGIS AES-128 ENGINE
 * Strategy: 4-way Interleaving + Sign-Bit GHASH
 */
struct AegisContext {
    __m128i round_keys[11];
};

/**
 * SIGN-BIT GHASH REDUCTION
 * Performs the GF(2^128) reduction without carry-flag branches.
 */
static inline __m128i ghash_reduce(__m128i x) {
    const __m128i poly = _mm_set_epi32(0, 0, 0, 0xE1000000);
    
    // Extract MSB to sign position
    // We treat the 128-bit register as 4x32-bit and shift
    __m128i top_bit = _mm_srai_epi32(x, 31); 
    __m128i mask = _mm_shuffle_epi32(top_bit, _MM_SHUFFLE(3, 3, 3, 3));
    
    // Branchless XOR reduction
    return _mm_xor_si128(_mm_slli_epi64(x, 1), _mm_and_si128(mask, poly));
}

/**
 * 4-WAY INTERLEAVED ENCRYPTION
 * Processes 64 bytes (4 blocks) simultaneously to hide AES-NI latency.
 */
static inline void encrypt_4way(AegisContext& ctx, __m128i* blocks) {
    // Round 0: Initial AddRoundKey
    for(int i=0; i<4; ++i) blocks[i] = _mm_xor_si128(blocks[i], ctx.round_keys[0]);

    // Rounds 1-9: The Interleaved Core
    // We process all 4 blocks for Round 1, then all 4 for Round 2...
    // This allows Block 0's Round 1 to finish while we work on Blocks 1, 2, 3.
    for (int r = 1; r < 10; ++r) {
        __m128i rk = ctx.round_keys[r];
        blocks[0] = _mm_aesenc_si128(blocks[0], rk);
        blocks[1] = _mm_aesenc_si128(blocks[1], rk);
        blocks[2] = _mm_aesenc_si128(blocks[2], rk);
        blocks[3] = _mm_aesenc_si128(blocks[3], rk);
    }

    // Round 10: Final Round
    __m128i rk10 = ctx.round_keys[10];
    for(int i=0; i<4; ++i) blocks[i] = _mm_aesenclast_si128(blocks[i], rk10);
}

} // namespace aegis
#endif
