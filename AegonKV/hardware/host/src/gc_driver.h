//
// Created by 冯昊 on 2023/9/1.
//

#ifndef TITAN_HARDWARE_GC_DRIVER_H
#define TITAN_HARDWARE_GC_DRIVER_H

#pragma once
#include "defines.h"

class GCDriver {
 public:
  GCDriver(const std::string& binaryFile, uint8_t device_id);
  ~GCDriver();

  static size_t get_file_size(const char* fileName) {
    if (fileName == nullptr) {
      return 0;
    }
    struct stat stat_buf {};
    stat(fileName, &stat_buf);
    size_t filesize = stat_buf.st_size;
    return filesize;
  }

  void run_gc_kernel(
      const std::vector<std::string>& input_filenames,
      const std::string& output_filename,
      const std::vector<std::pair<size_t, unsigned char*>>& bitmaps,
      const std::vector<std::uint64_t>& input_entries, uint8_t cu_index,
      std::vector<std::pair<std::string, std::vector<uint32_t>>>* rewrite_keys,
      std::vector<uint64_t>& output_meta);

 private:
  cl::Program* m_program;
  cl::Context* m_context;
  cl::CommandQueue* m_q;

  std::vector<std::string> gc_kernel_names = {
      "gcKernel:{gcKernel_1}", "gcKernel:{gcKernel_2}", "gcKernel:{gcKernel_3}",
      "gcKernel:{gcKernel_4}", "gcKernel:{gcKernel_5}", "gcKernel:{gcKernel_6}",
      "gcKernel:{gcKernel_7}", "gcKernel:{gcKernel_8}"};

  std::pair<std::string, std::vector<uint32_t>> GetOutputKey(
      unsigned char* data, uint64_t* cur_offset);
  static bool DecodeVarint32(const unsigned char* p, uint32_t& value,
                             uint64_t& offset) {
    uint32_t result = 0;
    offset = 0;
    for (uint32_t shift = 0; shift <= 28 && offset < 5; shift += 7) {
      uint32_t byte = *p;
      p++;
      offset++;
      if (byte & 128) {
        // More bytes are present
        result |= ((byte & 127) << shift);
      } else {
        result |= (byte << shift);
        value = result;
        return true;
      }
    }
    return false;
  }
  static inline void DecodeFixed32(const unsigned char* input,
                                   uint32_t* value) {
    memcpy(value, input, sizeof(uint32_t));
  }
};

#endif  // TITAN_HARDWARE_GC_DRIVER_H
