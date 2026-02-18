#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <numeric>
#include "../include/aegis/aegis_engine.hpp"

using namespace std;
using namespace std::chrono;

void run_timing_trial(const string& label, const aegis::AegisContext& ctx, __m128i val) {
    const size_t iterations = 1000000;
    alignas(16) __m128i block = val;
    
    auto start = high_resolution_clock::now();
    for(size_t i = 0; i < iterations; ++i) {
        // Use the safe wrapper to ensure no branches are taken based on data
        aegis::encrypt(ctx, &block, 1);
        // Force the compiler to not optimize away the loop
        asm volatile("" : "+m" (block));
    }
    auto end = high_resolution_clock::now();
    
    duration<double, std::nano> diff = end - start;
    cout << label << ": " << fixed << setprecision(2) 
         << (diff.count() / iterations) << " ns/block" << endl;
}

int main() {
    aegis::AegisContext ctx;
    ctx.rounds = 10;
    for(int i=0; i<11; ++i) ctx.round_keys[i] = _mm_set1_epi32(i);

    cout << "--- AEGIS Constant-Time Side-Channel Audit ---" << endl;
    cout << "Iterations: 1,000,000 per trial" << endl;

    // Trial 1: All Zeros (Simplest data)
    run_timing_trial("Zero Data  ", ctx, _mm_setzero_si128());

    // Trial 2: High Entropy (Noisy data)
    run_timing_trial("Random Data", ctx, _mm_set_epi32(0x12345678, 0x9ABCDEF0, 0xDEADBEEF, 0xCAFEBABE));

    cout << "----------------------------------------------" << endl;
    cout << "If the times are nearly identical, the algorithm is constant-time." << endl;

    return 0;
}
