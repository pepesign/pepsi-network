// keccak.c
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>
// A baseline Keccak (3rd round) implementation.

// Memory-hard extension of keccak for PoW 
// Copyright (c) 2014 The Boolberry developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "wild_keccak.h"
namespace crypto
{

  const uint64_t keccakf_rndc[24] = 
  {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080, 
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008
  };

  const int keccakf_rotc[24] = 
  {
    1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14, 
    27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
  };

  const int keccakf_piln[24] = 
  {
    10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4, 
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1 
  };

  // update the state with given number of rounds
  void regular_f::keccakf(uint64_t st[25], int rounds)
  {
    int i, j, round;
    uint64_t t, bc[5];

    for (round = 0; round < rounds; round++) {

      // Theta
      for (i = 0; i < 5; i++)     
        bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

      for (i = 0; i < 5; i++) {
        t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
        for (j = 0; j < 25; j += 5)
          st[j + i] ^= t;
      }

      // Rho Pi
      t = st[1];
      for (i = 0; i < 24; i++) {
        j = keccakf_piln[i];
        bc[0] = st[j];
        st[j] = ROTL64(t, keccakf_rotc[i]);
        t = bc[0];
      }

      //  Chi
      for (j = 0; j < 25; j += 5) {
        for (i = 0; i < 5; i++)
          bc[i] = st[j + i];
        for (i = 0; i < 5; i++)
          st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
      }

      //  Iota
      st[0] ^= keccakf_rndc[round];
    }
  }

  void mul_f::keccakf(uint64_t st[25], int rounds)
  {
    int i, j, round;
    uint64_t t, bc[5];

    for (round = 0; round < rounds; round++) {

      // Theta
      for (i = 0; i < 5; i++)     
      {
        bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] * st[i + 15] * st[i + 20];//surprise
      }

      for (i = 0; i < 5; i++) {
        t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
        for (j = 0; j < 25; j += 5)
          st[j + i] ^= t;
      }

      // Rho Pi
      t = st[1];
      for (i = 0; i < 24; i++) {
        j = keccakf_piln[i];
        bc[0] = st[j];
        st[j] = ROTL64(t, keccakf_rotc[i]);
        t = bc[0];
      }

      //  Chi
      for (j = 0; j < 25; j += 5) {
        for (i = 0; i < 5; i++)
          bc[i] = st[j + i];
        for (i = 0; i < 5; i++)
          st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
      }

      //  Iota
      st[0] ^= keccakf_rndc[round];
    }
  }
  bool generate_scratchpad(const crypto::hash& seed_data, std::vector<crypto::hash>& result_data, uint64_t target_size)
  {
    //this is very basic implementation, not considered for production (possible to reduce memory by keeping only every x10 item and calc it instead of read)
    //TODO: research safe way for scratchpad generation
    result_data.resize(target_size);
    result_data[0] = crypto::cn_fast_hash(&seed_data, sizeof(seed_data));
    //crypto::hash = get_transaction_hash()
    for (size_t i = 1; i < target_size; i++)
    {
      result_data[i] = crypto::cn_fast_hash(&result_data[i - 1], sizeof(result_data[i - 1]));
    }
    return true;
  }

#define WK2_COUNT 9

  bool generate_scratchpad2(const crypto::hash& seed_data, std::vector<crypto::hash>& result_data, uint64_t target_size)
  {
    //this is very basic implementation, not considered for production (possible to reduce memory by keeping only every x10 item and calc it instead of read)
    //TODO: research safe way for scratchpad generation
    result_data.resize(target_size);
    result_data[0] = crypto::cn_fast_hash(&seed_data, sizeof(seed_data));
    //crypto::hash = get_transaction_hash()
    for (size_t i = 1; i < target_size; i++)
    {
      result_data[i] = crypto::cn_fast_hash(&result_data[i - 1], sizeof(result_data[i - 1]));
      for (size_t j = 0; j != WK2_COUNT; j++)
      {
        result_data[i] = crypto::cn_fast_hash(&result_data[i], sizeof(result_data[i]));
      }      
    }
    return true;
  }
}

