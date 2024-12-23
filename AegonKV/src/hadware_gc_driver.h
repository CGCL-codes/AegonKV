//
// Created by 冯昊 on 2023/9/1.
//

#ifndef TITAN_HARDWARE_GC_DRIVER_H
#define TITAN_HARDWARE_GC_DRIVER_H

#pragma once
#include <fcntl.h> /* For O_RDWR */
#include <sys/stat.h>
#include <unistd.h> /* For open(), creat() */

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "blob_validation_check.h"
#include "port/port_posix.h"
#include "titan/options.h"
#include "titan_stats.h"
#include "xcl2.h"

namespace rocksdb {
namespace titandb {

#ifndef OUTPUT_KEY_SIZE
#define OUTPUT_KEY_SIZE (4 * 5 + 24)
#endif

#ifndef OUTPUT_META_SIZE
#define OUTPUT_META_SIZE (4)
#endif

class HardwareGCDriver {
 public:
  HardwareGCDriver(TitanDBOptions db_options, const std::string& binaryFile,
                   uint8_t device_id);
  ~HardwareGCDriver();

  void run_gc_kernel(
      const std::vector<std::string>& input_filenames,
      const std::string& output_filename,
      const std::vector<std::unique_ptr<BitMap>>& bitmaps,
      const std::vector<std::uint64_t>& input_entries, uint8_t cu_index,
      std::vector<std::pair<std::string, std::vector<uint32_t>>>* rewrite_keys,
      std::vector<uint64_t>& hardware_statistics,
      std::vector<uint64_t>& output_meta);

  Status get_free_cu_index(uint8_t& result);

  Status set_free_cu_index(uint8_t& index);

 private:
  TitanDBOptions db_options_;
  port::Mutex inner_mutex_;

  cl::Program* m_program_;
  cl::Context* m_context_;
  cl::CommandQueue* m_q_;

  std::vector<std::string> gc_kernel_names_ = {
      "Index Not Implemented", "gcKernel:{gcKernel_1}",
      "gcKernel:{gcKernel_2}", "gcKernel:{gcKernel_3}",
      "gcKernel:{gcKernel_4}", "gcKernel:{gcKernel_5}",
      "gcKernel:{gcKernel_6}", "gcKernel:{gcKernel_7}",
      "gcKernel:{gcKernel_8}"};
  std::queue<uint8_t> free_index_queue_;

  std::pair<std::string, std::vector<uint32_t>> GetOutputKey(
      const char* data, uint64_t* cur_offset);

  static size_t get_file_size(const char* fileName) {
    if (fileName == nullptr) {
      return 0;
    }
    struct stat stat_buf {};
    stat(fileName, &stat_buf);
    size_t filesize = stat_buf.st_size;
    return filesize;
  }
};

}  // namespace titandb
}  // namespace rocksdb

#endif  // TITAN_HARDWARE_GC_DRIVER_H
