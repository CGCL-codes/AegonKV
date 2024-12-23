
#include "titandb_db.h"

#include <rocksdb/convenience.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>
#include <rocksdb/utilities/options_util.h>
#include <titan/statistics.h>

#include <chrono>
#include <cinttypes>
#include <experimental/iterator>
#include <iomanip>
#include <memory>

#include "core/core_workload.h"
#include "core/db_factory.h"
#include "core/properties.h"
#include "core/utils.h"

namespace {
const std::string PROP_NAME = "titandb.dbname";
const std::string PROP_NAME_DEFAULT = "";

const std::string PROP_FORMAT = "titandb.format";
const std::string PROP_FORMAT_DEFAULT = "single";

const std::string PROP_MERGEUPDATE = "titandb.mergeupdate";
const std::string PROP_MERGEUPDATE_DEFAULT = "false";

const std::string PROP_DESTROY = "titandb.destroy";
const std::string PROP_DESTROY_DEFAULT = "false";

const std::string PROP_COMPRESSION = "titandb.compression";
const std::string PROP_COMPRESSION_DEFAULT = "no";

const std::string PROP_MAX_BG_COMPACTIONS =
    "titandb.max_background_compactions";
const std::string PROP_MAX_BG_COMPACTIONS_DEFAULT = "0";

const std::string PROP_MAX_BG_FLUSHES = "titandb.max_background_flushes";
const std::string PROP_MAX_BG_FLUSHES_DEFAULT = "0";

const std::string PROP_DISABLE_WAL = "titandb.disable_wal";
const std::string PROP_DISABLE_WAL_DEFAULT = "false";

const std::string DISABLE_GC = "titandb.disable_background_gc";
const std::string DISABLE_GC_DEFAULT = "false";

const std::string PROP_TARGET_FILE_SIZE_BASE = "titandb.target_file_size_base";
const std::string PROP_TARGET_FILE_SIZE_BASE_DEFAULT = "0";

const std::string PROP_TARGET_FILE_SIZE_MULT =
    "titandb.target_file_size_multiplier";
const std::string PROP_TARGET_FILE_SIZE_MULT_DEFAULT = "0";

const std::string PROP_MAX_BYTES_FOR_LEVEL_BASE =
    "titandb.max_bytes_for_level_base";
const std::string PROP_MAX_BYTES_FOR_LEVEL_BASE_DEFAULT = "0";

const std::string PROP_MAX_BYTES_FOR_LEVEL_MULTIPLIER =
    "titandb.max_bytes_for_level_multiplier";
const std::string PROP_MAX_BYTES_FOR_LEVEL_MULTIPLIER_DEFAULT = "10";

const std::string PROP_WRITE_BUFFER_SIZE = "titandb.write_buffer_size";
const std::string PROP_WRITE_BUFFER_SIZE_DEFAULT = "0";

const std::string PROP_MAX_WRITE_BUFFER = "titandb.max_write_buffer_number";
const std::string PROP_MAX_WRITE_BUFFER_DEFAULT = "0";

const std::string PROP_COMPACTION_PRI = "titandb.compaction_pri";
const std::string PROP_COMPACTION_PRI_DEFAULT = "-1";

const std::string PROP_MAX_OPEN_FILES = "titandb.max_open_files";
const std::string PROP_MAX_OPEN_FILES_DEFAULT = "-1";

const std::string PROP_L0_COMPACTION_TRIGGER =
    "titandb.level0_file_num_compaction_trigger";
const std::string PROP_L0_COMPACTION_TRIGGER_DEFAULT = "0";

const std::string PROP_L0_SLOWDOWN_TRIGGER =
    "titandb.level0_slowdown_writes_trigger";
const std::string PROP_L0_SLOWDOWN_TRIGGER_DEFAULT = "0";

const std::string PROP_L0_STOP_TRIGGER = "titandb.level0_stop_writes_trigger";
const std::string PROP_L0_STOP_TRIGGER_DEFAULT = "0";

const std::string PROP_USE_DIRECT_WRITE =
    "titandb.use_direct_io_for_flush_compaction";
const std::string PROP_USE_DIRECT_WRITE_DEFAULT = "false";

const std::string PROP_USE_DIRECT_READ = "titandb.use_direct_reads";
const std::string PROP_USE_DIRECT_READ_DEFAULT = "false";

const std::string PROP_USE_MMAP_WRITE = "titandb.allow_mmap_writes";
const std::string PROP_USE_MMAP_WRITE_DEFAULT = "false";

const std::string PROP_USE_MMAP_READ = "titandb.allow_mmap_reads";
const std::string PROP_USE_MMAP_READ_DEFAULT = "false";

const std::string PROP_CACHE_SIZE = "titandb.cache_size";
const std::string PROP_CACHE_SIZE_DEFAULT = "0";

const std::string PROP_COMPRESSED_CACHE_SIZE = "titandb.compressed_cache_size";
const std::string PROP_COMPRESSED_CACHE_SIZE_DEFAULT = "0";

const std::string PROP_BLOOM_BITS = "titandb.bloom_bits";
const std::string PROP_BLOOM_BITS_DEFAULT = "-1";

const std::string PROP_INCREASE_PARALLELISM = "titandb.increase_parallelism";
const std::string PROP_INCREASE_PARALLELISM_DEFAULT = "false";

const std::string PROP_OPTIMIZE_LEVELCOMP =
    "titandb.optimize_level_style_compaction";
const std::string PROP_OPTIMIZE_LEVELCOMP_DEFAULT = "false";

const std::string PROP_OPTIONS_FILE = "titandb.optionsfile";
const std::string PROP_OPTIONS_FILE_DEFAULT = "";

const std::string PROP_ENV_URI = "titandb.env_uri";
const std::string PROP_ENV_URI_DEFAULT = "";

const std::string PROP_FS_URI = "titandb.fs_uri";
const std::string PROP_FS_URI_DEFAULT = "";

//  blob props
const std::string PROP_TITAN_MAX_BACKGROUND_GC =
    "titandb.titan_max_background_gc";
const std::string PROP_TITAN_MAX_BACKGROUND_GC_DEFAULT = "1";

const std::string PROP_BLOB_FILE_DISCARDABLE_RATIO =
    "titandb.blob_file_discardable_ratio";
const std::string PROP_BLOB_FILE_DISCARDABLE_RATIO_DEFAULT = "0.5";

const std::string PROP_BLOB_DB_FILE_SIZE = "titandb.blob_db_file_size";
const std::string PROP_BLOB_DB_FILE_SIZE_DEFAULT = "268435456";

const std::string PROP_TITAN_MIN_BLOB_SIZE = "titandb.titan_min_blob_size";
const std::string PROP_TITAN_MIN_BLOB_SIZE_DEFAULT = "4096";

const std::string PROP_MAX_GC_BATCH_SIZE = "titandb.max_gc_batch_size";
const std::string PROP_MAX_GC_BATCH_SIZE_DEFAULT = "1073741824";

const std::string PROP_MIN_GC_BATCH_SIZE = "titandb.min_gc_batch_size";
const std::string PROP_MIN_GC_BATCH_SIZE_DEFAULT = "134217728";

const std::string PROP_TITAN_GC_TIME_PATH = "titandb.titan_gc_time_path";
const std::string PROP_TITAN_GC_TIME_PATH_DEFAULT = "";

const std::string PROP_LEVEL_MERGE = "titandb.level_merge";
const std::string PROP_LEVEL_MERGE_DEFAULT = "false";

const std::string PROP_RANGE_MERGE = "titandb.range_merge";
const std::string PROP_RANGE_MERGE_DEFAULT = "false";

const std::string PROP_MAX_SORTED_RUNS = "titandb.max_sorted_runs";
const std::string PROP_MAX_SORTED_RUNS_DEFAULT = "20";

const std::string PROP_USE_BITMAP = "titandb.use_bitmap";
const std::string PROP_USE_BITMAP_DEFAULT = "false";

const std::string PROP_GC_OFFLOAD = "titandb.gc_offload";
const std::string PROP_GC_OFFLOAD_DEFAULT = "false";

const std::string PROP_BINARY_FILE_PATH = "titandb.binary_file_path";
const std::string PROP_BINARY_FILE_PATH_DEFAULT = "";

const std::string PROP_ENABLE_PIPELINED_WRITE =
    "titandb.enable_pipelined_write";
const std::string PROP_ENABLE_PIPELINED_WRITE_DEFAULT = "false";

const std::string PROP_UNORDERED_WRITE = "titandb.unordered_write";
const std::string PROP_UNORDERED_WRITE_DEFAULT = "false";

const std::string PROP_ENABLE_MULTI_BATCH_WRITE =
    "titandb.enable_multi_batch_write";
const std::string PROP_ENABLE_MULTI_BATCH_WRITE_DEFAULT = "false";

static class std::shared_ptr<rocksdb::Statistics> dbstats;

static std::shared_ptr<rocksdb::Env> env_guard;
static std::shared_ptr<rocksdb::Cache> block_cache;
#if ROCKSDB_MAJOR < 8
static std::shared_ptr<rocksdb::Cache> block_cache_compressed;
#endif
}  // namespace

namespace ycsbc {

rocksdb::titandb::TitanDB *TitandbDB::db_ = nullptr;
int TitandbDB::ref_cnt_ = 0;
std::mutex TitandbDB::mu_;

void TitandbDB::Init() {
// merge operator disabled by default due to link error
#ifdef USE_MERGEUPDATE
  class YCSBUpdateMerge : public rocksdb::AssociativeMergeOperator {
   public:
    virtual bool Merge(const rocksdb::Slice &key,
                       const rocksdb::Slice *existing_value,
                       const rocksdb::Slice &value, std::string *new_value,
                       rocksdb::Logger *logger) const override {
      assert(existing_value);

      std::vector<Field> values;
      const char *p = existing_value->data();
      const char *lim = p + existing_value->size();
      DeserializeRow(values, p, lim);

      std::vector<Field> new_values;
      p = value.data();
      lim = p + value.size();
      DeserializeRow(new_values, p, lim);

      for (Field &new_field : new_values) {
        bool found = false;
        for (Field &field : values) {
          if (field.name == new_field.name) {
            found = true;
            field.value = new_field.value;
            break;
          }
        }
        if (!found) {
          values.push_back(new_field);
        }
      }

      SerializeRow(values, *new_value);
      return true;
    }

    virtual const char *Name() const override { return "YCSBUpdateMerge"; }
  };
#endif
  const std::lock_guard<std::mutex> lock(mu_);

  const utils::Properties &props = *props_;
  const std::string format =
      props.GetProperty(PROP_FORMAT, PROP_FORMAT_DEFAULT);
  if (format == "single") {
    format_ = kSingleRow;
    method_read_ = &TitandbDB::ReadSingle;
    method_scan_ = &TitandbDB::ScanSingle;
    method_update_ = &TitandbDB::UpdateSingle;
    method_insert_ = &TitandbDB::InsertSingle;
    method_delete_ = &TitandbDB::DeleteSingle;
#ifdef USE_MERGEUPDATE
    if (props.GetProperty(PROP_MERGEUPDATE, PROP_MERGEUPDATE_DEFAULT) ==
        "true") {
      method_update_ = &RocksdbDB::MergeSingle;
    }
#endif
  } else {
    throw utils::Exception("unknown format");
  }
  fieldcount_ = std::stoi(props.GetProperty(CoreWorkload::FIELD_COUNT_PROPERTY,
                                            CoreWorkload::FIELD_COUNT_DEFAULT));

  ref_cnt_++;
  if (db_) {
    return;
  }

  const std::string &db_path = props.GetProperty(PROP_NAME, PROP_NAME_DEFAULT);
  if (db_path == "") {
    throw utils::Exception("TitanDB db path is missing");
  }

  rocksdb::titandb::TitanOptions opt;
  opt.create_if_missing = true;
  GetOptions(props, &opt);
#ifdef USE_MERGEUPDATE
  opt.merge_operator.reset(new YCSBUpdateMerge);
#endif

  rocksdb::Status s;
  if (props.GetProperty(PROP_DESTROY, PROP_DESTROY_DEFAULT) == "true") {
    s = rocksdb::DestroyDB(db_path, opt);
    if (!s.ok()) {
      throw utils::Exception(std::string("Titan DestroyDB: ") + s.ToString());
    }
  }

  //  s = rocksdb::DB::Open(opt, db_path, &db_);
  rocksdb::titandb::TitanDB *ptr;
  s = rocksdb::titandb::TitanDB::Open(opt, db_path, &ptr);
  if (s.ok()) {
    db_ = ptr;
  }

  if (!s.ok()) {
    throw utils::Exception(std::string("RocksDB Open: ") + s.ToString());
  }
}

void TitandbDB::Cleanup() {
  const std::lock_guard<std::mutex> lock(mu_);
  if (--ref_cnt_) {
    return;
  }
  delete db_;
}

void TitandbDB::Statistics() {
  db_->WaitBackgroundJob();
  std::string res;
  res.reserve(20000);
  const int buffer_size = 200;
  for (const auto &t : rocksdb::TickersNameMap) {
    assert(t.first < rocksdb::TICKER_ENUM_MAX);
    char buffer[buffer_size];
    snprintf(buffer, buffer_size, "%s COUNT : %" PRIu64 "\n", t.second.c_str(),
             dbstats->getTickerCount(t.first));
    res.append(buffer);
  }
  for (const auto &t : rocksdb::titandb::TitanTickersNameMap) {
    assert(t.first < rocksdb::titandb::TITAN_TICKER_ENUM_MAX);
    char buffer[buffer_size];
    snprintf(buffer, buffer_size, "%s COUNT : %" PRIu64 "\n", t.second.c_str(),
             dbstats->getTickerCount(t.first));
    res.append(buffer);
  }
  for (const auto &h : rocksdb::HistogramsNameMap) {
    assert(h.first < rocksdb::HISTOGRAM_ENUM_MAX);
    char buffer[buffer_size];
    rocksdb::HistogramData hData;
    dbstats->histogramData(h.first, &hData);
    // don't handle failures - buffer should always be big enough and
    // arguments should be provided correctly
    int ret =
        snprintf(buffer, buffer_size,
                 "%s P50 : %f P95 : %f P99 : %f P100 : %f COUNT : %" PRIu64
                 " SUM : %" PRIu64 "\n",
                 h.second.c_str(), hData.median, hData.percentile95,
                 hData.percentile99, hData.max, hData.count, hData.sum);
    if (ret < 0 || ret >= buffer_size) {
      assert(false);
      continue;
    }
    res.append(buffer);
  }
  for (const auto &h : rocksdb::titandb::TitanHistogramsNameMap) {
    assert(h.first < rocksdb::titandb::TITAN_HISTOGRAM_ENUM_MAX);
    char buffer[buffer_size];
    rocksdb::HistogramData hData;
    dbstats->histogramData(h.first, &hData);
    // don't handle failures - buffer should always be big enough and
    // arguments should be provided correctly
    int ret =
        snprintf(buffer, buffer_size,
                 "%s P50 : %f P95 : %f P99 : %f P100 : %f COUNT : %" PRIu64
                 " SUM : %" PRIu64 "\n",
                 h.second.c_str(), hData.median, hData.percentile95,
                 hData.percentile99, hData.max, hData.count, hData.sum);
    if (ret < 0 || ret >= buffer_size) {
      assert(false);
      continue;
    }
    res.append(buffer);
  }
  res.shrink_to_fit();
  fprintf(stdout, "STATISTICS:\n%s\n", res.c_str());
  dbstats->Reset();
}

void TitandbDB::GetOptions(const utils::Properties &props,
                           rocksdb::titandb::TitanOptions *opt) {
  std::string env_uri = props.GetProperty(PROP_ENV_URI, PROP_ENV_URI_DEFAULT);
  std::string fs_uri = props.GetProperty(PROP_FS_URI, PROP_FS_URI_DEFAULT);
  rocksdb::Env *env = rocksdb::Env::Default();

  if (!env_uri.empty() || !fs_uri.empty()) {
    rocksdb::Status s = rocksdb::Env::CreateFromUri(
        rocksdb::ConfigOptions(), env_uri, fs_uri, &env, &env_guard);
    if (!s.ok()) {
      throw utils::Exception(std::string("RocksDB CreateFromUri: ") +
                             s.ToString());
    }
    opt->env = env;
  }

  const std::string options_file =
      props.GetProperty(PROP_OPTIONS_FILE, PROP_OPTIONS_FILE_DEFAULT);
  if (options_file != "") {
    rocksdb::ConfigOptions config_options;
    config_options.ignore_unknown_options = false;
    config_options.input_strings_escaped = true;
    config_options.env = env;
    rocksdb::Options tmp_opts;
    std::vector<rocksdb::ColumnFamilyDescriptor> tmp_cf_descs;
    rocksdb::Status s = rocksdb::LoadOptionsFromFile(
        config_options, options_file, &tmp_opts, &tmp_cf_descs);
    if (!s.ok()) {
      throw utils::Exception(std::string("RocksDB LoadOptionsFromFile: ") +
                             s.ToString());
    }
    *opt = rocksdb::titandb::TitanOptions(tmp_opts);
  } else {
    const std::string compression_type =
        props.GetProperty(PROP_COMPRESSION, PROP_COMPRESSION_DEFAULT);
    if (compression_type == "no") {
      opt->compression = rocksdb::kNoCompression;
    } else if (compression_type == "snappy") {
      opt->compression = rocksdb::kSnappyCompression;
    } else if (compression_type == "zlib") {
      opt->compression = rocksdb::kZlibCompression;
    } else if (compression_type == "bzip2") {
      opt->compression = rocksdb::kBZip2Compression;
    } else if (compression_type == "lz4") {
      opt->compression = rocksdb::kLZ4Compression;
    } else if (compression_type == "lz4hc") {
      opt->compression = rocksdb::kLZ4HCCompression;
    } else if (compression_type == "xpress") {
      opt->compression = rocksdb::kXpressCompression;
    } else if (compression_type == "zstd") {
      opt->compression = rocksdb::kZSTD;
    } else {
      throw utils::Exception("Unknown compression type");
    }

    //    int val = std::stoi(props.GetProperty(PROP_MAX_BG_JOBS,
    //    PROP_MAX_BG_JOBS_DEFAULT)); if (val != 0) {
    //      opt->max_background_jobs = val;
    //    }

    int val = std::stoi(props.GetProperty(PROP_MAX_BG_COMPACTIONS,
                                          PROP_MAX_BG_COMPACTIONS_DEFAULT));
    if (val != 0) {
      opt->max_background_compactions = val;
    }
    val = std::stoi(
        props.GetProperty(PROP_MAX_BG_FLUSHES, PROP_MAX_BG_FLUSHES_DEFAULT));
    if (val != 0) {
      opt->max_background_flushes = val;
    }
    val = std::stoi(props.GetProperty(PROP_TARGET_FILE_SIZE_BASE,
                                      PROP_TARGET_FILE_SIZE_BASE_DEFAULT));
    if (val != 0) {
      opt->target_file_size_base = val;
    }
    val = std::stoi(props.GetProperty(PROP_TARGET_FILE_SIZE_MULT,
                                      PROP_TARGET_FILE_SIZE_MULT_DEFAULT));
    if (val != 0) {
      opt->target_file_size_multiplier = val;
    }
    val = std::stoi(props.GetProperty(PROP_MAX_BYTES_FOR_LEVEL_BASE,
                                      PROP_MAX_BYTES_FOR_LEVEL_BASE_DEFAULT));
    if (val != 0) {
      opt->max_bytes_for_level_base = val;
    }
    val = std::stoi(
        props.GetProperty(PROP_MAX_BYTES_FOR_LEVEL_MULTIPLIER,
                          PROP_MAX_BYTES_FOR_LEVEL_MULTIPLIER_DEFAULT));
    if (val != 0) {
      opt->max_bytes_for_level_multiplier = val;
    }
    val = std::stoi(props.GetProperty(PROP_WRITE_BUFFER_SIZE,
                                      PROP_WRITE_BUFFER_SIZE_DEFAULT));
    if (val != 0) {
      opt->write_buffer_size = val;
    }
    val = std::stoi(props.GetProperty(PROP_MAX_WRITE_BUFFER,
                                      PROP_MAX_WRITE_BUFFER_DEFAULT));
    if (val != 0) {
      opt->max_write_buffer_number = val;
    }
    val = std::stoi(
        props.GetProperty(PROP_COMPACTION_PRI, PROP_COMPACTION_PRI_DEFAULT));
    if (val != -1) {
      opt->compaction_pri = static_cast<rocksdb::CompactionPri>(val);
    }
    val = std::stoi(
        props.GetProperty(PROP_MAX_OPEN_FILES, PROP_MAX_OPEN_FILES_DEFAULT));
    if (val != 0) {
      opt->max_open_files = val;
    }

    val = std::stoi(props.GetProperty(PROP_L0_COMPACTION_TRIGGER,
                                      PROP_L0_COMPACTION_TRIGGER_DEFAULT));
    if (val != 0) {
      opt->level0_file_num_compaction_trigger = val;
    }
    val = std::stoi(props.GetProperty(PROP_L0_SLOWDOWN_TRIGGER,
                                      PROP_L0_SLOWDOWN_TRIGGER_DEFAULT));
    if (val != 0) {
      opt->level0_slowdown_writes_trigger = val;
    }
    val = std::stoi(
        props.GetProperty(PROP_L0_STOP_TRIGGER, PROP_L0_STOP_TRIGGER_DEFAULT));
    if (val != 0) {
      opt->level0_stop_writes_trigger = val;
    }

    if (props.GetProperty(PROP_USE_DIRECT_WRITE,
                          PROP_USE_DIRECT_WRITE_DEFAULT) == "true") {
      opt->use_direct_io_for_flush_and_compaction = true;
    }
    if (props.GetProperty(PROP_USE_DIRECT_READ, PROP_USE_DIRECT_READ_DEFAULT) ==
        "true") {
      opt->use_direct_reads = true;
    }
    if (props.GetProperty(PROP_USE_MMAP_WRITE, PROP_USE_MMAP_WRITE_DEFAULT) ==
        "true") {
      opt->allow_mmap_writes = true;
    }
    if (props.GetProperty(PROP_USE_MMAP_READ, PROP_USE_MMAP_READ_DEFAULT) ==
        "true") {
      opt->allow_mmap_reads = true;
    }

    if (props.GetProperty("statistics", "false") == "true") {
      dbstats = rocksdb::titandb::CreateDBStatistics();
      dbstats->set_stats_level(static_cast<rocksdb::StatsLevel>(
          rocksdb::StatsLevel::kExceptDetailedTimers));
      opt->statistics = dbstats;
    }
    if (props.GetProperty(PROP_DISABLE_WAL, PROP_DISABLE_WAL_DEFAULT) ==
        "true") {
      write_options_.disableWAL = true;
    }

    if (props.GetProperty(DISABLE_GC, DISABLE_GC_DEFAULT) ==
        "true") {
      opt->disable_background_gc = true;
    }

    val = std::stoi(props.GetProperty(PROP_TITAN_MAX_BACKGROUND_GC,
                                      PROP_TITAN_MAX_BACKGROUND_GC_DEFAULT));
    opt->max_background_gc = val;

    double d_val =
        std::stod(props.GetProperty(PROP_BLOB_FILE_DISCARDABLE_RATIO,
                                    PROP_BLOB_FILE_DISCARDABLE_RATIO_DEFAULT));
    opt->blob_file_discardable_ratio = d_val;

    val = std::stoi(props.GetProperty(PROP_BLOB_DB_FILE_SIZE,
                                      PROP_BLOB_DB_FILE_SIZE_DEFAULT));
    opt->blob_file_target_size = val;

    val = std::stoi(props.GetProperty(PROP_TITAN_MIN_BLOB_SIZE,
                                      PROP_TITAN_MIN_BLOB_SIZE_DEFAULT));
    opt->min_blob_size = val;

    val = std::stoi(props.GetProperty(PROP_MAX_GC_BATCH_SIZE,
                                      PROP_MAX_GC_BATCH_SIZE_DEFAULT));
    opt->max_gc_batch_size = val;

    val = std::stoi(props.GetProperty(PROP_MIN_GC_BATCH_SIZE,
                                      PROP_MIN_GC_BATCH_SIZE_DEFAULT));
    opt->min_gc_batch_size = val;

    //    opt->level_compaction_dynamic_level_bytes = true;

    if (props.GetProperty(PROP_LEVEL_MERGE, PROP_LEVEL_MERGE_DEFAULT) ==
        "true") {
      opt->level_merge = true;
      opt->level_compaction_dynamic_level_bytes = true;
    }

    if (props.GetProperty(PROP_RANGE_MERGE, PROP_RANGE_MERGE_DEFAULT) ==
        "true") {
      opt->range_merge = true;
    }

    val = std::stoi(
        props.GetProperty(PROP_MAX_SORTED_RUNS, PROP_MAX_SORTED_RUNS_DEFAULT));
    opt->max_sorted_runs = val;

    if (props.GetProperty(PROP_USE_BITMAP, PROP_USE_BITMAP_DEFAULT) == "true") {
      opt->use_bitmap = true;
    }

    if (props.GetProperty(PROP_GC_OFFLOAD, PROP_GC_OFFLOAD_DEFAULT) == "true") {
      opt->gc_offload = true;
    }

    opt->binary_file_path =
        props.GetProperty(PROP_BINARY_FILE_PATH, PROP_BINARY_FILE_PATH_DEFAULT);

    if (props.GetProperty(PROP_ENABLE_PIPELINED_WRITE,
                          PROP_ENABLE_PIPELINED_WRITE_DEFAULT) == "true") {
      opt->enable_pipelined_write = true;
    }

    if (props.GetProperty(PROP_UNORDERED_WRITE, PROP_UNORDERED_WRITE_DEFAULT) ==
        "true") {
      opt->unordered_write = true;
    }

    if (props.GetProperty(PROP_ENABLE_MULTI_BATCH_WRITE,
                          PROP_ENABLE_MULTI_BATCH_WRITE_DEFAULT) == "true") {
      opt->enable_multi_batch_write = true;
    }

    rocksdb::BlockBasedTableOptions table_options;
    size_t cache_size =
        std::stoul(props.GetProperty(PROP_CACHE_SIZE, PROP_CACHE_SIZE_DEFAULT));
    if (cache_size > 0) {
      block_cache = rocksdb::NewLRUCache(cache_size, 6, false, 0.0);
      table_options.block_cache = block_cache;
    }
#if ROCKSDB_MAJOR < 8
    size_t compressed_cache_size = std::stoul(props.GetProperty(
        PROP_COMPRESSED_CACHE_SIZE, PROP_COMPRESSED_CACHE_SIZE_DEFAULT));
    if (compressed_cache_size > 0) {
      block_cache_compressed = rocksdb::NewLRUCache(compressed_cache_size);
      table_options.block_cache_compressed = block_cache_compressed;
    }
#endif
    int bloom_bits =
        std::stoul(props.GetProperty(PROP_BLOOM_BITS, PROP_BLOOM_BITS_DEFAULT));
    if (bloom_bits > 0) {
      table_options.filter_policy.reset(
          rocksdb::NewBloomFilterPolicy(bloom_bits));
    }
    opt->table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));

    if (props.GetProperty(PROP_INCREASE_PARALLELISM,
                          PROP_INCREASE_PARALLELISM_DEFAULT) == "true") {
      opt->IncreaseParallelism();
    }
    if (props.GetProperty(PROP_OPTIMIZE_LEVELCOMP,
                          PROP_OPTIMIZE_LEVELCOMP_DEFAULT) == "true") {
      opt->OptimizeLevelStyleCompaction();
    }
  }
}

void TitandbDB::SerializeRow(const std::vector<Field> &values,
                             std::string &data) {
  for (const Field &field : values) {
    uint32_t len = field.name.size();
    data.append(reinterpret_cast<char *>(&len), sizeof(uint32_t));
    data.append(field.name.data(), field.name.size());
    len = field.value.size();
    data.append(reinterpret_cast<char *>(&len), sizeof(uint32_t));
    data.append(field.value.data(), field.value.size());
  }
}

void TitandbDB::DeserializeRowFilter(std::vector<Field> &values, const char *p,
                                     const char *lim,
                                     const std::vector<std::string> &fields) {
  std::vector<std::string>::const_iterator filter_iter = fields.begin();
  while (p != lim && filter_iter != fields.end()) {
    assert(p < lim);
    uint32_t len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    std::string field(p, static_cast<const size_t>(len));
    p += len;
    len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    std::string value(p, static_cast<const size_t>(len));
    p += len;
    if (*filter_iter == field) {
      values.push_back({field, value});
      filter_iter++;
    }
  }
  assert(values.size() == fields.size());
}

void TitandbDB::DeserializeRowFilter(std::vector<Field> &values,
                                     const std::string &data,
                                     const std::vector<std::string> &fields) {
  const char *p = data.data();
  const char *lim = p + data.size();
  DeserializeRowFilter(values, p, lim, fields);
}

void TitandbDB::DeserializeRow(std::vector<Field> &values, const char *p,
                               const char *lim) {
  while (p != lim) {
    assert(p < lim);
    uint32_t len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    std::string field(p, static_cast<const size_t>(len));
    p += len;
    len = *reinterpret_cast<const uint32_t *>(p);
    p += sizeof(uint32_t);
    std::string value(p, static_cast<const size_t>(len));
    p += len;
    values.push_back({field, value});
  }
}

void TitandbDB::DeserializeRow(std::vector<Field> &values,
                               const std::string &data) {
  const char *p = data.data();
  const char *lim = p + data.size();
  DeserializeRow(values, p, lim);
}

DB::Status TitandbDB::ReadSingle(const std::string &table,
                                 const std::string &key,
                                 const std::vector<std::string> *fields,
                                 std::vector<Field> &result) {
  std::string data;
  rocksdb::Status s = db_->Get(read_options_, key, &data);
  if (s.IsNotFound()) {
    return kNotFound;
  } else if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Get: ") + s.ToString());
  }
  if (fields != nullptr) {
    DeserializeRowFilter(result, data, *fields);
  } else {
    DeserializeRow(result, data);
//    assert(result.size() == static_cast<size_t>(fieldcount_));
  }
  return kOK;
}

DB::Status TitandbDB::ScanSingle(const std::string &table,
                                 const std::string &key, int len,
                                 const std::vector<std::string> *fields,
                                 std::vector<std::vector<Field>> &result) {
  rocksdb::Iterator *db_iter = db_->NewIterator(read_options_);
  db_iter->Seek(key);
  for (int i = 0; db_iter->Valid() && i < len; i++) {
    std::string data = db_iter->value().ToString();
    result.push_back(std::vector<Field>());
    std::vector<Field> &values = result.back();
    if (fields != nullptr) {
      DeserializeRowFilter(values, data, *fields);
    } else {
      DeserializeRow(values, data);
//      assert(values.size() == static_cast<size_t>(fieldcount_));
    }
    db_iter->Next();
  }
  delete db_iter;
  return kOK;
}

DB::Status TitandbDB::UpdateSingle(const std::string &table,
                                   const std::string &key,
                                   std::vector<Field> &values) {
  std::string data;
  rocksdb::Status s = db_->Get(read_options_, key, &data);
  if (s.IsNotFound()) {
    return kNotFound;
  } else if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Get: ") + s.ToString());
  }
  std::vector<Field> current_values;
  DeserializeRow(current_values, data);
  assert(current_values.size() == static_cast<size_t>(fieldcount_));
  for (Field &new_field : values) {
    bool found MAYBE_UNUSED = false;
    for (Field &cur_field : current_values) {
      if (cur_field.name == new_field.name) {
        found = true;
        cur_field.value = new_field.value;
        break;
      }
    }
    assert(found);
  }

  data.clear();
  SerializeRow(current_values, data);
  s = db_->Put(write_options_, key, data);
  if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Put: ") + s.ToString());
  }
  return kOK;
}

DB::Status TitandbDB::MergeSingle(const std::string &table,
                                  const std::string &key,
                                  std::vector<Field> &values) {
  std::string data;
  SerializeRow(values, data);
  rocksdb::Status s = db_->Merge(write_options_, key, data);
  if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Merge: ") + s.ToString());
  }
  return kOK;
}

DB::Status TitandbDB::InsertSingle(const std::string &table,
                                   const std::string &key,
                                   std::vector<Field> &values) {
  std::string data;
  SerializeRow(values, data);
  rocksdb::Status s = db_->Put(write_options_, key, data);
  if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Put: ") + s.ToString());
  }
  return kOK;
}

DB::Status TitandbDB::DeleteSingle(const std::string &table,
                                   const std::string &key) {
  rocksdb::Status s = db_->Delete(write_options_, key);
  if (!s.ok()) {
    throw utils::Exception(std::string("TitanDB Delete: ") + s.ToString());
  }
  return kOK;
}

void TitandbDB::GetGCTimeList(std::vector<std::vector<uint64_t>> *result) {
  db_->GetGCTimeStats(result);
}

void TitandbDB::OnTransactionFinished() {
  db_->WaitBackgroundJob();
  std::vector<std::vector<uint64_t>> result;
  GetGCTimeList(&result);

  std::string path = props_->GetProperty(PROP_TITAN_GC_TIME_PATH,
                                         PROP_TITAN_GC_TIME_PATH_DEFAULT);
  if (!path.empty()) {
    std::ofstream ofstream_gc_split(path,
                                    std::ios::app | std::ofstream::binary);
    ofstream_gc_split << "start time,end time,read lsm micros,update lsm "
                         "micros,read blob micros,write blob micros,read lsm "
                         "num,read blob num,write back "
                         "num,hardware_gc_bytes_read,hardware_gc_read_blob_"
                         "micros,hardware_gc_kernel_micros,hardware_gc_bytes_"
                         "written,hardware_gc_write_blob_micros"
                      << std::endl;
    for (const auto &item : result) {
      std::copy(item.begin(), item.end(),
                std::experimental::make_ostream_joiner(ofstream_gc_split, ","));
      ofstream_gc_split << std::endl;
    }
    ofstream_gc_split.close();
  }
}

DB *NewTitandbDB() { return new TitandbDB; }

const bool registered = DBFactory::RegisterDB("titandb", NewTitandbDB);

}  // namespace ycsbc
