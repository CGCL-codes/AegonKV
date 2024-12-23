//
// Created by 冯昊 on 2023/7/27.
//

#include "blob_validation_check.h"

#include "test_util/testharness.h"

namespace rocksdb {
namespace titandb {

class BlobValidationCheckTest : public testing::Test {
 public:
  void SetBit(BitMap* bitmap, uint64_t bitoffset, int on) {
    ASSERT_OK(bitmap->SetBit(bitoffset, on));
  }

  void GetBit(BitMap* bitmap, uint64_t bitoffset, int* result) {
    ASSERT_OK(bitmap->GetBit(bitoffset, result));
  }

  void OrBit(BitMap* bm1, BitMap* bm2, BitMap::BitOpsType type) {}

  int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
  }

  // 虚拟内存和物理内存，单位为kb
  typedef struct {
    uint32_t virtualMem;
    uint32_t physicalMem;
  } processMem_t;

  processMem_t GetProcessMemory() {
    FILE* file = fopen("/proc/self/status", "r");
    char line[128];
    processMem_t processMem;

    while (fgets(line, 128, file) != nullptr) {
      if (strncmp(line, "VmSize:", 7) == 0) {
        processMem.virtualMem = parseLine(line);
        break;
      }

      if (strncmp(line, "VmRSS:", 6) == 0) {
        processMem.physicalMem = parseLine(line);
        break;
      }
    }
    fclose(file);
    return processMem;
  }
};

TEST_F(BlobValidationCheckTest, BitMapBasic) {
  BitMap* bitmaps[1000];
  for (int i = 0; i < 1000; ++i) {
    bitmaps[i] = new BitMap(65535);
    ASSERT_TRUE(bitmaps[i]);
    SetBit(bitmaps[i], i, 1);
    SetBit(bitmaps[i], i + 1, 1);
    SetBit(bitmaps[i], i + 1, 0);
    int result = 0;
    GetBit(bitmaps[i], i, &result);
    ASSERT_TRUE(result == 1);
    GetBit(bitmaps[i], i + 1, &result);
    ASSERT_TRUE(result == 0);
  }
  for (const auto& item : bitmaps) {
    delete item;
  }
}

TEST_F(BlobValidationCheckTest, BitOps) {
  {
    // or
    BitMap bm1(10000), bm2(10000), bm3(10001);
    for (int i = 0; i < 10000; ++i) {
      SetBit(&bm1, i, i % 2);
      SetBit(&bm2, i, (i + 1) % 2);
    }
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitOr, nullptr));
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitOr, &bm3));
    ASSERT_OK(bm1.BitOps(BitMap::BitOpsType::kBitOr, &bm2));
    for (int i = 0; i < 1000; ++i) {
      int result = 0;
      GetBit(&bm1, i, &result);
      ASSERT_TRUE(result == 1);
    }
  }
  {
    // and
    BitMap bm1(10000), bm2(10000), bm3(10001);
    for (int i = 0; i < 10000; ++i) {
      SetBit(&bm1, i, i % 2);
      SetBit(&bm2, i, (i + 1) % 2);
    }
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitAnd, nullptr));
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitAnd, &bm3));
    ASSERT_OK(bm1.BitOps(BitMap::BitOpsType::kBitAnd, &bm2));
    for (int i = 0; i < 10000; ++i) {
      int result = 0;
      GetBit(&bm1, i, &result);
      ASSERT_TRUE(result == 0);
    }
  }
  {
    // xor
    BitMap bm1(10000), bm2(10000), bm3(10001), bm4(10000);
    for (int i = 0; i < 10000; ++i) {
      SetBit(&bm1, i, i % 2);
      SetBit(&bm2, i, (i + 1) % 2);
      SetBit(&bm4, i, (i + 1) % 2);
    }
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitXor, nullptr));
    ASSERT_NOK(bm1.BitOps(BitMap::BitOpsType::kBitXor, &bm3));
    ASSERT_OK(bm1.BitOps(BitMap::BitOpsType::kBitXor, &bm2));
    ASSERT_OK(bm2.BitOps(BitMap::BitOpsType::kBitXor, &bm4));
    for (int i = 0; i < 10000; ++i) {
      int result = 0;
      GetBit(&bm1, i, &result);
      ASSERT_TRUE(result == 1);
      GetBit(&bm2, i, &result);
      ASSERT_TRUE(result == 0);
    }
  }
}

TEST_F(BlobValidationCheckTest, Copy) {
  auto bm1 = std::unique_ptr<BitMap>(new BitMap(100000));
  for (int i = 0; i < 100000; ++i) {
    SetBit(bm1.get(), i, i % 2);
  }
  std::unique_ptr<BitMap> bm2(new BitMap());
  Status s = bm1->CopyTo(bm2.get());
  ASSERT_OK(s);
  for (int i = 0; i < 100000; ++i) {
    int result = 0;
    GetBit(bm2.get(), i, &result);
    ASSERT_TRUE(result == (i % 2));
  }
  s = bm1->CopyTo(bm2.get());
  ASSERT_NOK(s);
}

TEST_F(BlobValidationCheckTest, BlobValidationChecker) {}

}  // namespace titandb
}  // namespace rocksdb

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}