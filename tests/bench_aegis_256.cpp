#include <iostream>
#include <chrono>
#include <iomanip>
#include <immintrin.h>
#include "../include/aegis/aegis_engine.hpp"

// Forward declaration from our src file
namespace aegis { void expand_key_256(const uint8_t* key, AegisContext& ctx); }

int main() {
    const size_t num_blocks = 1024 * 64; // 1MB
    __m128i* data = (__m128i*)std::aligned_alloc(16, num_blocks * sizeof(__m128i));
    uint8_t dummy_key[32] = {0};
    
    aegis::AegisContext ctx;
    aegis::expand_key_256(dummy_key, ctx);

    std::cout << "--- AEGIS-256 Performance Audit ---" << std::endl;
    std::cout << "Target: 14 Rounds, 4-way Interleave" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < num_blocks; i += 4) {
        aegis::encrypt_4way(ctx, &data[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> d = end - start;
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "AES-256 Time: " << d.count() << "s" << std::endl;
    std::cout << "Performance:  " << (1.0 / d.count()) << " MB/s" << std::endl;

    std::free(data);
    return 0;
}
