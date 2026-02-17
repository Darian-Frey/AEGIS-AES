# AEGIS-AES 🛡️
**Architectural ID:** ST-AEGIS-2026
**Protocol:** SCHEMA_V5
**Performance Milestone:** 397% Throughput Gain (4-way Interleaved)

## 1. Abstract
AEGIS is a high-speed AES-128-GCM engine optimized for x86_64 processors. Standard AES-NI implementations are often latency-bound by the 4-cycle turnaround of the `aesenc` instruction. AEGIS utilizes **Horizontal Interleaving** and **Sign-Bit GHASH Reduction** to saturate the CPU's execution ports, achieving near-theoretical peak throughput.

## 2. Technical Methodology

### A. 4-way Latency Hiding
By processing 4 blocks of ciphertext in parallel, AEGIS ensures that the AES execution unit never stalls. While Block 0 is in the middle of a 4-cycle round, the unit is already processing the first cycles of Blocks 1, 2, and 3.

### B. Sign-Bit GHASH (Branchless)
Standard GHASH reduction requires checking the carry bit for polynomial XORing. AEGIS utilizes the **Neptune Sign-Bit Bridge**:
- Extract MSB to sign position.
- Arithmetic shift right (`srai`) to create a 0xFF or 0x00 mask.
- XOR with the polynomial constant branchlessly.

## 3. Performance Results (Latitude 5480 / i5-7300U)
- **Standard AES-NI:** 0.0041s (Latency Bound)
- **AEGIS 4-way:** 0.0008s (Throughput Bound)
- **Throughput Gain:** **397.3%**

## 4. Usage
AEGIS is a header-only library. Include `include/aegis/aegis_engine.hpp` and compile with `-maes`.

## 5. License
MIT - Part of the SCHEMA_V5 High-Performance Suite.
