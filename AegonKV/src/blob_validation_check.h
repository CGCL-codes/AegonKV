//
// Created by 冯昊 on 2023/7/20.
//

#include <memory>
#include <unordered_map>

#include "rocksdb/slice.h"
#include "rocksdb/status.h"
#ifndef TITAN_BLOB_VALIDATION_CHECK_H

namespace rocksdb {
namespace titandb {

class BitMap {
 public:
  // init a new bitmap
  explicit BitMap(size_t validBits);

  BitMap();

  ~BitMap();

  BitMap(BitMap&& bitMap) noexcept;

  Status CopyTo(BitMap* dest);

  Status SetBit(uint64_t bitoffset, int on);

  Status GetBit(uint64_t bitoffset, int* result);

  enum class BitOpsType {
    kBitAnd,
    kBitOr,
    kBitXor,
    // not implemented: kBitNot
  };

  Status BitOps(BitOpsType type, BitMap* other);

  //  Status BitCount(int *result);

 private:
  friend class BlobGCJob;
  friend class HardwareGCDriver;

  size_t valid_bits_;
  size_t byte_size_;
  unsigned char* bits_;
};

// todo bitmap memory to storage, consistency support
// bitmap file format
//
//  <begin>
//  [header]
//  [meta]
//  [bit data]
//  <end>
//
class BitMapStorage {
 public:
  Status Flush();

  Status Get();

 private:
};

}  // namespace titandb
}  // namespace rocksdb

#define TITAN_BLOB_VALIDATION_CHECK_H

#endif  // TITAN_BLOB_VALIDATION_CHECK_H
