#pragma once

#include "blob_file_builder.h"
#include "blob_file_iterator.h"
#include "blob_file_manager.h"
#include "blob_file_set.h"
#include "blob_gc.h"
#include "db/db_impl/db_impl.h"
#include "hadware_gc_driver.h"
#include "rocksdb/statistics.h"
#include "rocksdb/status.h"
#include "titan/options.h"
#include "titan_stats.h"
#include "version_edit.h"

namespace rocksdb {
namespace titandb {

class BlobGCJob {
 public:
  BlobGCJob(BlobGC *blob_gc, DB *db, port::Mutex *mutex,
            const TitanDBOptions &titan_db_options, Env *env,
            const EnvOptions &env_options, BlobFileManager *blob_file_manager,
            BlobFileSet *blob_file_set, LogBuffer *log_buffer,
            std::atomic_bool *shuting_down, TitanStats *stats,
            port::Mutex *gc_mutex,
            std::shared_ptr<HardwareGCDriver> hardware_gc_driver = nullptr);

  // No copying allowed
  BlobGCJob(const BlobGCJob &) = delete;
  void operator=(const BlobGCJob &) = delete;

  ~BlobGCJob();

  // REQUIRE: mutex held
  Status Prepare();
  // REQUIRE: mutex not held
  Status Run();
  // REQUIRE: mutex held
  Status Finish();

  void get_gc_metrics(std::vector<uint64_t> *) const;

 private:
  class GarbageCollectionWriteCallback;
  class BlobFileMergeIteratorWithReadStats;
  friend class BlobGCJobTest;

  void UpdateInternalOpStats();

  BlobGC *blob_gc_;
  DB *base_db_;
  DBImpl *base_db_impl_;
  port::Mutex *mutex_;
  TitanDBOptions db_options_;
  Env *env_;
  EnvOptions env_options_;
  BlobFileManager *blob_file_manager_;
  BlobFileSet *blob_file_set_;
  LogBuffer *log_buffer_{nullptr};

  std::vector<std::pair<std::unique_ptr<BlobFileHandle>,
                        std::unique_ptr<BlobFileBuilder>>>
      blob_file_builders_;
  std::vector<std::pair<WriteBatch, GarbageCollectionWriteCallback>>
      rewrite_batches_;

  std::atomic_bool *shuting_down_{nullptr};

  TitanStats *stats_;

  struct {
    uint64_t gc_bytes_read = 0;
    uint64_t gc_bytes_written = 0;
    uint64_t gc_num_keys_overwritten = 0;
    uint64_t gc_bytes_overwritten = 0;
    uint64_t gc_num_keys_relocated = 0;
    uint64_t gc_bytes_relocated = 0;
    uint64_t gc_num_keys_fallback = 0;
    uint64_t gc_bytes_fallback = 0;
    uint64_t gc_num_new_files = 0;
    uint64_t gc_num_files = 0;
    uint64_t gc_read_lsm_micros = 0;
    uint64_t gc_update_lsm_micros = 0;
    uint64_t gc_read_blob_micros = 0;
    uint64_t gc_write_blob_micros = 0;
    uint64_t gc_num_read_lsm = 0;
    uint64_t gc_num_read_blob = 0;
    uint64_t gc_num_write_back = 0;
  } metrics_;

  struct {
    uint64_t hardware_gc_bytes_read = 0;
    uint64_t hardware_gc_read_blob_micros = 0;
    uint64_t hardware_gc_kernel_micros = 0;
    uint64_t hardware_gc_bytes_written = 0;
    uint64_t hardware_gc_write_blob_micros = 0;
  } hardware_metrics_;

  uint64_t prev_bytes_read_ = 0;
  uint64_t prev_bytes_written_ = 0;
  uint64_t io_bytes_read_ = 0;
  uint64_t io_bytes_written_ = 0;

  bool use_bitmap_{false};
  bool gc_offload_{false};
  std::shared_ptr<HardwareGCDriver> hardware_gc_driver_;
  std::shared_ptr<BlobFileMeta> hardware_blob_file_;
  port::Mutex *gc_mutex_;

  Status DoRunGC();
  void BatchWriteNewIndices(BlobFileBuilder::OutContexts &contexts, Status *s);
  Status BuildIterator(std::unique_ptr<BlobFileMergeIterator> *result);
  Status DiscardEntry(const Slice &key, const BlobIndex &blob_index,
                      bool *discardable);
  Status InstallOutputBlobFiles();
  Status InstallHardwareOutputBlobFiles();
  Status RewriteValidKeyToLSM();
  Status DeleteInputBlobFiles();

  Status DoRunGCOnHardware();

  bool IsShutingDown();
};

}  // namespace titandb
}  // namespace rocksdb
