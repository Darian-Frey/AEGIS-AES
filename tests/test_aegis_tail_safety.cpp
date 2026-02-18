#include <iostream>
#include <vector>
#include <cassert>
#include "../include/aegis/aegis_engine.hpp"

int main() {
    std::cout << "--- AEGIS-AES Tail Safety Audit ---" << std::endl;
    aegis::AegisContext ctx;
    ctx.rounds = 10;
    for(int i=0; i<11; ++i) ctx.round_keys[i] = _mm_set1_epi32(i);

    const size_t num_blocks = 7; // Non-multiple of 4
    std::vector<__m128i> data(num_blocks, _mm_set1_epi32(0xABCDEF));

    // This would have smashed the stack before; now it should be safe.
    aegis::encrypt(ctx, data.data(), num_blocks);

    std::cout << "SUCCESS: Processed 7 blocks without crash." << std::endl;
    return 0;
}
