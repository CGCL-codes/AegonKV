//
// Created by 冯昊 on 2023/8/29.
//

#ifndef TITAN_HARDWARE_GC_HELPER_H
#define TITAN_HARDWARE_GC_HELPER_H

#include "coding.h"
#include "constant.h"

namespace xf {
namespace gc {

inline bool DecodeHeader(const unsigned char *data, uint64_t *cur_offset) {
  data += *cur_offset;
  unsigned char head_buf[kBlobMaxHeaderSize];
  for (int i = 0; i < kBlobMaxHeaderSize; ++i) {
#pragma HLS pipeline ii = 1
    head_buf[i] = data[i];
  }

  uint32_t magic_number = 0, version = 0;
  DecodeFixed32(head_buf + 0, &magic_number);
  if (magic_number != kHeaderMagicNumber) {
    return false;
  }
  DecodeFixed32(head_buf + 4, &version);
  if (version != kHeaderVersion1 && version != kHeaderVersion2) {
    return false;
  }
  if (version == kHeaderVersion2) {
    // Check that no other flags are set
    uint32_t flags = 0;
    DecodeFixed32(head_buf + 8, &flags);
    if (flags & ~kHasUncompressedDictionary) {
      return false;
    }
    *cur_offset += kBlobMaxHeaderSize;
  } else {
    *cur_offset += kBlobMinHeaderSize;
  }
  return true;
}

bool DecodeFooter(const unsigned char *data, uint64_t data_size) {
  unsigned char footer_buf[kBlobFooterSize];
  for (int i = 0; i < kBlobFooterSize; ++i) {
#pragma HLS pipeline ii = 1
    footer_buf[i] = data[data_size - kBlobFooterSize + i];
  }

  uint64_t var_offset_result = 0;
  uint64_t offset = 0, size = 0;
  DecodeVarint64(footer_buf, footer_buf + kBlobFooterSize, offset,
                 var_offset_result);
  DecodeVarint64(footer_buf + var_offset_result, footer_buf + kBlobFooterSize,
                 size, var_offset_result);
  uint64_t magic_number = 0;
  DecodeFixed64(footer_buf + kBlobFooterSize - 12, &magic_number);
  if (magic_number != kFooterMagicNumber) {
    return false;
  }
  uint32_t checksum = 0;
  DecodeFixed32(footer_buf + kBlobFooterSize - 4, &checksum);

  return true;
}

inline void EncodeHeader(unsigned char *data, uint64_t *cur_offset) {
  data += *cur_offset;
  EncodeFixed32(data, kHeaderMagicNumber);
  EncodeFixed32(data + 4, kHeaderVersion2);
  EncodeFixed32(data + 8, 0);
  *cur_offset += kBlobMaxHeaderSize;
}

void EncodeFooter(unsigned char *data, uint64_t *cur_offset) {
  uint32_t align_padding = ((*cur_offset) + kBlobFooterSize) & 4095;
  if (align_padding > 0) {
    align_padding = 4096 - align_padding;
  }
  *cur_offset += align_padding;
  data += *cur_offset;
  uint64_t var_offset = 0;
  EncodeVarint64(data, 0, var_offset);
  EncodeVarint64(data + var_offset, 0, var_offset);
  EncodeFixed64(data + kBlobFooterSize - 12, kFooterMagicNumber);
  uint32_t crc;
  Crc32c(0, data, kBlobFooterSize - 4, &crc);
  EncodeFixed32(data + kBlobFooterSize - 4, crc);
  *cur_offset += kBlobFooterSize;
}

inline bool CheckValid(uint64_t offset, const unsigned char *bitmap) {
  uint64_t byte, bit;
  uint64_t bitval;

  byte = offset >> 3;
  bit = 7 - (offset & 0x7);
  bitval = ((uint8_t *)bitmap)[byte] & (1 << bit);

  return bitval == 0;
}

inline void SetZero(unsigned char *arr, int size) {
  for (int i = 0; i < size; ++i) {
#pragma HLS pipeline ii = 1
    *(arr + i) = (char)0;
  }
}

void GetKV(const unsigned char *data, uint64_t *cur_offset, bool valid,
           unsigned char *key, uint32_t &key_size, unsigned char *value,
           uint32_t &value_size) {
  data += *cur_offset;

  //  unsigned char record_header_buf[kRecordHeaderSize];
  //  for (int i = 0; i < kRecordHeaderSize; ++i) {
  // #pragma HLS pipeline ii = 1
  //    record_header_buf[i] = *(data + i);
  //  }
  //  uint32_t crc = 0, record_size = 0;
  //  DecodeFixed32(record_header_buf, &crc);
  //  DecodeFixed32(record_header_buf + 4, &record_size);

  //  unsigned char compression = *(record_header_buf + 8);
  //  uint32_t crc_compute =
  //      Crc32c(0, data + 4, kRecordHeaderSize - 4 + record_size);

  data += kRecordHeaderSize;
  *cur_offset += kRecordHeaderSize;

  uint64_t key_offset = 0, value_offset = 0;
  if (valid) {
    DecodeVarint32AndValue(data, key, key_size, key_offset);
    DecodeVarint32AndValue(data + key_offset, value, value_size, value_offset);
  } else {
    DecodeVarint32(data, data + 5, key_size, key_offset);
    key_offset += key_size;
    DecodeVarint32(data + key_offset, data + key_offset + 5, value_size,
                   value_offset);
    value_offset += value_size;
  }
  *cur_offset += key_offset + value_offset;
}

void PutOutputData(unsigned char *data, unsigned char *key, uint32_t key_size,
                   unsigned char *value, uint32_t value_size,
                   uint64_t *cur_offset) {
  data += *cur_offset;
  uint64_t key_offset = 0, value_offset = 0;
  EncodeVarint32AndValue(data + kRecordHeaderSize, key, key_size, key_offset);
  EncodeVarint32AndValue(data + kRecordHeaderSize + key_offset, value,
                         value_size, value_offset);
  EncodeFixed32(data + 4, key_offset + value_offset);
  *(data + 8) = 0;
  uint32_t crc;
  Crc32c(0, data + 4, kRecordHeaderSize - 4 + key_offset + value_offset, &crc);
  EncodeFixed32(data, crc);
  *cur_offset += kRecordHeaderSize + key_offset + value_offset;
}

void PutOutputKey(unsigned char *data, unsigned char *key, uint32_t key_size,
                  uint64_t *cur_offset, uint64_t new_data_offset,
                  uint64_t original_file, uint64_t original_data_offset,
                  uint64_t entry_size) {
  data += *cur_offset;
  uint64_t offset = 0;
  EncodeVarint32(data, key_size, offset);
  for (int i = 0; i < key_size; ++i) {
#pragma HLS pipeline ii = 1
    data[offset + i] = key[i];
  }
  EncodeFixed32(data + offset + key_size, new_data_offset);
  EncodeFixed32(data + offset + key_size + 4, original_file);
  EncodeFixed32(data + offset + key_size + 8, original_data_offset);
  EncodeFixed32(data + offset + key_size + 12, entry_size);
  *cur_offset += offset + key_size + 16;
}

}  // namespace gc
}  // namespace xf

#endif  // TITAN_HARDWARE_GC_HELPER_H
