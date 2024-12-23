//
// Created by 冯昊 on 2023/8/30.
//

#ifndef TITAN_HARDWARE_CODING_H
#define TITAN_HARDWARE_CODING_H

#include <ap_int.h>

#include "constant.h"
#include "crc32c.h"
#include "hls_stream.h"

namespace xf {
namespace gc {

inline void DecodeFixed32(const unsigned char* input, uint32_t* value) {
  uint32_t result = 0;
  for (int i = 0; i < 4; ++i) {
    uint32_t byte = *input;
    input++;
    result |= (byte << (8 * i));
    *value = result;
  }
}

inline void DecodeFixed64(const unsigned char* input, uint64_t* value) {
  uint64_t result = 0;
  for (int i = 0; i < 8; ++i) {
    uint64_t byte = *input;
    input++;
    result |= (byte << (8 * i));
    *value = result;
  }
}

inline void EncodeFixed32(unsigned char* buf, uint32_t value) {
  static const unsigned int B = 256;
  for (int i = 0; i < 4; ++i) {
    *(buf++) = (value & (B - 1)) | B;
    value >>= 8;
  }
}

inline void EncodeFixed64(unsigned char* buf, uint64_t value) {
  static const unsigned int B = 256;
  for (int i = 0; i < 8; ++i) {
    *(buf++) = (value & (B - 1)) | B;
    value >>= 8;
  }
}

void DecodeVarint64(const unsigned char* p, const unsigned char* limit,
                    uint64_t& value, uint64_t& offset) {
  uint64_t result = 0;
  for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
    uint64_t byte = *p;
    p++;
    offset++;
    if (byte & 128) {
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      value = result;
      break;
    }
  }
}

void EncodeVarint64(unsigned char* dst, uint64_t v, uint64_t& offset) {
  static const unsigned int B = 128;
  while (v >= B) {
    *(dst++) = (v & (B - 1)) | B;
    v >>= 7;
    offset++;
  }
  *(dst++) = static_cast<unsigned char>(v);
  offset++;
}

void DecodeVarint32(const unsigned char* p, const unsigned char* limit,
                    uint32_t& value, uint64_t& offset) {
  uint32_t result = 0;
  for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
    uint32_t byte = *p;
    p++;
    offset++;
    if (byte & 128) {
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      value = result;
      break;
    }
  }
}

void EncodeVarint32(unsigned char* dst, uint32_t v, uint64_t& offset) {
  static const int B = 128;
  if (v < (1 << 7)) {
    *(dst++) = v;
    offset += 1;
  } else if (v < (1 << 14)) {
    *(dst++) = v | B;
    *(dst++) = v >> 7;
    offset += 2;
  } else if (v < (1 << 21)) {
    *(dst++) = v | B;
    *(dst++) = (v >> 7) | B;
    *(dst++) = v >> 14;
    offset += 3;
  } else if (v < (1 << 28)) {
    *(dst++) = v | B;
    *(dst++) = (v >> 7) | B;
    *(dst++) = (v >> 14) | B;
    *(dst++) = v >> 21;
    offset += 4;
  } else {
    *(dst++) = v | B;
    *(dst++) = (v >> 7) | B;
    *(dst++) = (v >> 14) | B;
    *(dst++) = (v >> 21) | B;
    *(dst++) = v >> 28;
    offset += 5;
  }
}

inline void DecodeVarint32AndValue(const unsigned char* data,
                                   unsigned char* key, uint32_t& key_size,
                                   uint64_t& offset) {
  uint32_t len = 0;
  DecodeVarint32(data, data + 5, len, offset);
  key_size = len;
  data += offset;
  for (int i = 0; i < len; ++i) {
#pragma HLS pipeline ii = 1
    *(key + i) = *(data + i);
  }
  offset += len;
}

inline void EncodeVarint32AndValue(unsigned char* data, unsigned char* key,
                                   uint32_t key_size, uint64_t& offset) {
  EncodeVarint32(data, key_size, offset);
  for (int i = 0; i < key_size; ++i) {
#pragma HLS pipeline ii = 1
    *(data + offset + i) = *(key + i);
  }
  offset += key_size;
}

void Crc32c(uint32_t init_crc, const unsigned char* data, uint64_t n,
            uint32_t* result) {
#pragma HLS dataflow
  hls::stream<ap_uint<8> > dataStrm;
  hls::stream<ap_uint<32> > lenStrm;
  hls::stream<ap_uint<32> > crcInitStrm;
  hls::stream<bool> endStrm;

  hls::stream<ap_uint<32> > crc32Strm;
  hls::stream<bool> crc32EndStrm;

#pragma HLS stream variable = dataStrm depth = 16
#pragma HLS stream variable = lenStrm depth = 16
#pragma HLS stream variable = crcInitStrm depth = 16
#pragma HLS stream variable = endStrm depth = 16
#pragma HLS stream variable = crc32Strm depth = 16
#pragma HLS stream variable = crc32EndStrm depth = 16

  lenStrm.write(n);
  crcInitStrm.write(init_crc ^ 0xffffffffu);
  endStrm.write(false);
  endStrm.write(true);
  for (int i = 0; i < n; i++) {
#pragma HLS pipeline ii = 1
    dataStrm.write(ap_uint<8>(data[i]));
  }

  xf::security::crc32c<1>(crcInitStrm, dataStrm, lenStrm, endStrm, crc32Strm,
                          crc32EndStrm);

  crc32EndStrm.read();
  *result = crc32Strm.read();
  crc32EndStrm.read();
}

}  // namespace gc
}  // namespace xf

#endif  // TITAN_HARDWARE_CODING_H
