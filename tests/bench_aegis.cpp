#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <immintrin.h>
#include "../include/aegis/aegis_engine.hpp"

int main() {
    const size_t num_blocks = 1024 * 64; // 1MB total
    // Using aligned_alloc to ensure 16-byte alignment for AES-NI
    __m128i* data = (__m128i*)std::aligned_alloc(16, num_blocks * sizeof(__m128i));
    
    aegis::AegisContext ctx;
    // Mock Key Expansion
    for(int i=0; i<11; ++i) ctx.round_keys[i] = _mm_set1_epi32(i);

    std::cout << "--- AEGIS Latency Hiding Audit ---" << std::endl;

    // Test 1-way (Latency Bound)
    auto start1 = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < num_blocks; ++i) {
        __m128i tmp = _mm_xor_si128(data[i], ctx.round_keys[0]);
        for(int r=1; r<10; ++r) tmp = _mm_aesenc_si128(tmp, ctx.round_keys[r]);
        data[i] = _mm_aesenclast_si128(tmp, ctx.round_keys[10]);
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    // Test 4-way (Throughput Bound)
    auto start2 = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i < num_blocks; i += 4) {
        aegis::encrypt_4way(ctx, &data[i]);
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> d1 = end1 - start1;
    std::chrono::duration<double> d2 = end2 - start2;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Standard AES-NI: " << d1.count() << "s" << std::endl;
    std::cout << "AEGIS 4-way:     " << d2.count() << "s" << std::endl;
    std::cout << "Throughput Gain: " << (d1.count() / d2.count() - 1.0) * 100.0 << "%" << std::endl;

    free(data);
    return 0;
}
