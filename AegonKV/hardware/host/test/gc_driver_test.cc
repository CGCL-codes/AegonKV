//
// Created by 冯昊 on 2023/9/5.
//

#include "gc_driver.h"

#include <cinttypes>
#include <thread>

std::string GenKey(uint64_t i) {
  char buf[64];
  snprintf(buf, sizeof(buf), "k-%08" PRIu64, i);
  return buf;
}

std::string GenValue(uint64_t k) {
  if (k % 2 == 0) {
    return std::string(4096 - 1, 'v');
  } else {
    return std::string(4096 + 1, 'v');
  }
}

void ConcurrentGC(GCDriver *gcDriver, const std::string &input_file,
                  size_t entries, const std::string &output_file,
                  uint8_t cu_index) {
  std::vector<std::string> input_blob;
  input_blob.push_back(input_file);

  std::vector<std::pair<size_t, unsigned char *>> bitmaps;
  std::vector<std::uint64_t> input_entries;

  size_t bitmap_size = entries / 8 + 1;
  auto bitmap = (unsigned char *)aligned_alloc(4096, bitmap_size);
  memset(bitmap, 3, bitmap_size);
  bitmaps.emplace_back(bitmap_size, bitmap);
  input_entries.push_back(entries);

  std::vector<std::pair<std::string, std::vector<uint32_t>>> rewrite_keys;
  std::vector<uint64_t> hardware_statistics;
  std::vector<uint64_t> output_meta;

  gcDriver->run_gc_kernel(input_blob, output_file, bitmaps, input_entries,
                          cu_index, &rewrite_keys, output_meta);

  std::cout << "rewrite key size: " << rewrite_keys.size() << std::endl;
}

int main(int argc, char *argv[]) {
  std::string binary(
      "/home/hfeng/code/titan/cmake-build-debug-node27/hardware/kernel/hw/"
      "gc.xclbin");
  uint8_t device_id = 0;
  GCDriver driver(binary, device_id);

  bool concurrent = true;
  if (concurrent) {
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    for (int i = 0; i < 8; ++i) {
      std::string input_blob("/home/SmartSSD_data/hfeng/titan/2" +
                             std::to_string(i % 4) + ".blob");
      inputs.push_back(input_blob);
      std::string ouput_blob("/home/SmartSSD_data/hfeng/titan/4" +
                             std::to_string(i) + ".blob");
      outputs.push_back(ouput_blob);
    }
    std::thread t0(ConcurrentGC, &driver, inputs[0], 30000, outputs[0],
                   (uint8_t)0);
    std::thread t1(ConcurrentGC, &driver, inputs[1], 30000, outputs[1],
                   (uint8_t)1);
    std::thread t2(ConcurrentGC, &driver, inputs[2], 30000, outputs[2],
                   (uint8_t)2);
    std::thread t3(ConcurrentGC, &driver, inputs[3], 30000, outputs[3],
                   (uint8_t)3);
    std::thread t4(ConcurrentGC, &driver, inputs[4], 30000, outputs[4],
                   (uint8_t)4);
    std::thread t5(ConcurrentGC, &driver, inputs[5], 30000, outputs[5],
                   (uint8_t)5);
    std::thread t6(ConcurrentGC, &driver, inputs[6], 30000, outputs[6],
                   (uint8_t)6);
    std::thread t7(ConcurrentGC, &driver, inputs[7], 30000, outputs[7],
                   (uint8_t)7);
    t0.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    std::cout << 1 << std::endl;
  } else {
    std::vector<std::string> input_blob;
    input_blob.push_back("/home/SmartSSD_data/hfeng/titan/1.blob");
    input_blob.push_back("/home/SmartSSD_data/hfeng/titan/2.blob");
    input_blob.push_back("/home/SmartSSD_data/hfeng/titan/3.blob");

    std::string ouput_blob("/home/SmartSSD_data/hfeng/titan/4.blob");
    //  std::string ouput_blob("/home/SmartSSD_Data/hfeng/titan/14.blob");
    std::vector<std::pair<size_t, unsigned char *>> bitmaps;
    std::vector<std::uint64_t> input_entries;
    size_t bitmap_size = 30000 / 8 + 1;
    for (int i = 0; i < 3; ++i) {
      auto bitmap = (unsigned char *)aligned_alloc(4096, bitmap_size);
      //    memset(bitmap, 0, bitmap_size);
      memset(bitmap, 15, bitmap_size);
      bitmaps.emplace_back(bitmap_size, bitmap);
      input_entries.push_back(32);
    }

    std::vector<std::pair<std::string, std::vector<uint32_t>>> rewrite_keys;
    std::vector<uint64_t> output_meta;

    driver.run_gc_kernel(input_blob, ouput_blob, bitmaps, input_entries, 0,
                         &rewrite_keys, output_meta);

    std::cout << "rewrite key size: " << rewrite_keys.size() << std::endl;
    for (int i = 0; i < 10; ++i) {
      auto info = rewrite_keys[i];
      std::cout << info.first << ", ";
      for (const auto &item : info.second) {
        std::cout << item << ", ";
      }
      std::cout << std::endl;
    }
    //  assert(rewrite_keys.size() == 9);
  }
}
