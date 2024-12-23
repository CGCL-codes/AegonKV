#pragma once

#include "blob_file_set.h"
#include "db_impl.h"
#include "rocksdb/listener.h"
#include "rocksdb/table_properties.h"
#include "util/coding.h"

namespace rocksdb {
namespace titandb {

class BlobValidationCollectorFactory final
    : public TablePropertiesCollectorFactory {
 public:
  TablePropertiesCollector* CreateTablePropertiesCollector(
      TablePropertiesCollectorFactory::Context context) override;

  const char* Name() const override { return "BlobValidationCollector"; }
};

class BlobValidationCollector final : public TablePropertiesCollector {
 public:
  const static std::string kPropertiesName;

  static bool Encode(const std::unordered_map<uint64_t, std::vector<uint64_t>>&
                         blob_entry_indexes,
                     std::string* result);
  static bool Decode(Slice* slice,
                     std::unordered_map<uint64_t, std::vector<uint64_t>>* blob_entry_indexes);

  Status AddUserKey(const Slice& key, const Slice& value, EntryType type,
                    SequenceNumber seq, uint64_t file_size) override;
  Status Finish(UserCollectedProperties* properties) override;
  UserCollectedProperties GetReadableProperties() const override {
    return UserCollectedProperties();
  }
  const char* Name() const override { return "BlobValidationCollector"; }

 private:
  std::unordered_map<uint64_t, std::vector<uint64_t>> blob_invalid_entries_;
};

}  // namespace titandb
}  // namespace rocksdb
