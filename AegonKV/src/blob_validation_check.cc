//
// Created by 冯昊 on 2023/7/20.
//

#include "blob_validation_check.h"

// Bitmap
rocksdb::titandb::BitMap::BitMap(size_t validBits) : valid_bits_(validBits) {
  size_t byte = (validBits >> 3) + 1;
  byte_size_ = byte;
  bits_ = (unsigned char *)malloc(byte * sizeof(unsigned char));
  memset(bits_, 0, byte * sizeof(unsigned char));
}
rocksdb::titandb::BitMap::BitMap()
    : valid_bits_(0), byte_size_(0), bits_(nullptr) {}
rocksdb::titandb::BitMap::~BitMap() {
  if (bits_ != nullptr) {
    free(bits_);
    bits_ = nullptr;
  }
}
rocksdb::titandb::BitMap::BitMap(rocksdb::titandb::BitMap &&bitMap) noexcept
    : valid_bits_(0), byte_size_(0), bits_(bitMap.bits_) {
  bitMap.bits_ = nullptr;
}
rocksdb::Status rocksdb::titandb::BitMap::SetBit(uint64_t bitoffset, int on) {
  rocksdb::Status s;
  /* Bits can only be set or cleared... */
  if (on & ~1 || bitoffset > valid_bits_) {
    return rocksdb::Status::InvalidArgument(
        "bit is not an integer or out of range");
  }

  ssize_t byte, bit;
  int byteval, bitval;

  /* Get current values */
  byte = bitoffset >> 3;
  byteval = ((uint8_t *)bits_)[byte];
  bit = 7 - (bitoffset & 0x7);
  bitval = byteval & (1 << bit);

  /* Either it is newly created, changed length, or the bit changes before and
   * after. Note that the bitval here is actually a decimal number. So we need
   * to use `!!` to convert it to 0 or 1 for comparison. */
  if (!!bitval != on) {
    /* Update byte with new bit value. */
    byteval &= ~(1 << bit);
    byteval |= ((on & 0x1) << bit);
    ((uint8_t *)bits_)[byte] = byteval;
  }

  return s;
}
rocksdb::Status rocksdb::titandb::BitMap::GetBit(uint64_t bitoffset,
                                                 int *result) {
  rocksdb::Status s;
  if (bitoffset > valid_bits_) {
    return rocksdb::Status::InvalidArgument(
        "bit is not an integer or out of range");
  }

  size_t byte, bit;
  size_t bitval = 0;

  byte = bitoffset >> 3;
  bit = 7 - (bitoffset & 0x7);
  bitval = ((uint8_t *)bits_)[byte] & (1 << bit);

  *result = bitval ? 1 : 0;

  return s;
}
rocksdb::Status rocksdb::titandb::BitMap::BitOps(BitOpsType type,
                                                 BitMap *other) {
  rocksdb::Status s;
  if (other == nullptr) {
    return Status::InvalidArgument("bit operate with nullptr");
  }
  size_t aligned_remain_size, ops_size;
  if (valid_bits_ != other->valid_bits_) {
    return Status::InvalidArgument("valid bits not match");
  }
  aligned_remain_size =
      other->byte_size_ < byte_size_ ? other->byte_size_ : byte_size_;
  ops_size = aligned_remain_size;

  /* Compute the bit operation. */
  if (aligned_remain_size) {
    unsigned char output, byte;

    /* Fast path: as far as we have data for all the input bitmaps we
     * can take a fast path that performs much better than the
     * vanilla algorithm. On ARM we skip the fast path since it will
     * result in GCC compiling the code using multiple-words load/store
     * operations that are not supported even in ARM >= v6. */
    unsigned long j = 0;
    unsigned long *lp = (unsigned long *)other->bits_;
    unsigned long *lres = (unsigned long *)bits_;
#ifndef USE_ALIGNED_ACCESS
    if (aligned_remain_size >= sizeof(unsigned long) * 4) {
      /* Different branches per different operations for speed (sorry). */
      if (type == BitOpsType::kBitAnd) {
        while (aligned_remain_size >= sizeof(unsigned long) * 4) {
          lres[0] &= lp[0];
          lres[1] &= lp[1];
          lres[2] &= lp[2];
          lres[3] &= lp[3];
          lp += 4;

          lres += 4;
          j += sizeof(unsigned long) * 4;
          aligned_remain_size -= sizeof(unsigned long) * 4;
        }
      } else if (type == BitOpsType::kBitOr) {
        while (aligned_remain_size >= sizeof(unsigned long) * 4) {
          lres[0] |= lp[0];
          lres[1] |= lp[1];
          lres[2] |= lp[2];
          lres[3] |= lp[3];
          lp += 4;

          lres += 4;
          j += sizeof(unsigned long) * 4;
          aligned_remain_size -= sizeof(unsigned long) * 4;
        }
      } else if (type == BitOpsType::kBitXor) {
        while (aligned_remain_size >= sizeof(unsigned long) * 4) {
          lres[0] ^= lp[0];
          lres[1] ^= lp[1];
          lres[2] ^= lp[2];
          lres[3] ^= lp[3];
          lp += 4;

          lres += 4;
          j += sizeof(unsigned long) * 4;
          aligned_remain_size -= sizeof(unsigned long) * 4;
        }
      }
      //      else if (type == BitOpsType::kBitNot) {
      //        while (aligned_remain_size >= sizeof(unsigned long) * 4) {
      //          lres[0] = ~lres[0];
      //          lres[1] = ~lres[1];
      //          lres[2] = ~lres[2];
      //          lres[3] = ~lres[3];
      //          lres += 4;
      //          j += sizeof(unsigned long) * 4;
      //          aligned_remain_size -= sizeof(unsigned long) * 4;
      //        }
      //      }
    }
#endif

    /* j is set to the next byte to process by the previous loop. */
    for (; j < ops_size; j++) {
      output = bits_[j];
      byte = other->bits_[j];
      switch (type) {
        case BitOpsType::kBitAnd:
          output &= byte;
          break;
        case BitOpsType::kBitOr:
          output |= byte;
          break;
        case BitOpsType::kBitXor:
          output ^= byte;
          break;
          //        case BitOpsType::kBitNot:
          //          output = ~output;
          //          break;
      }
      bits_[j] = output;
    }
  }

  return s;
}
rocksdb::Status rocksdb::titandb::BitMap::CopyTo(
    rocksdb::titandb::BitMap *dest) {
  rocksdb::Status s;
  if (dest->bits_) {
    s = Status::Corruption("other bimap already been inited.");
    return s;
  }
  dest->valid_bits_ = valid_bits_;
  dest->byte_size_ = byte_size_;
  dest->bits_ = (unsigned char *)malloc(byte_size_ * sizeof(unsigned char));
  memset(dest->bits_, 0, byte_size_ * sizeof(unsigned char));
  memcpy(dest->bits_, bits_, byte_size_ * sizeof(unsigned char));
  return s;
}
