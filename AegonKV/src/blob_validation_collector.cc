#include "blob_validation_collector.h"

#include "base_db_listener.h"

namespace rocksdb {
namespace titandb {

TablePropertiesCollector*
BlobValidationCollectorFactory::CreateTablePropertiesCollector(
    rocksdb::TablePropertiesCollectorFactory::Context /* context */) {
  return new BlobValidationCollector();
}

const std::string BlobValidationCollector::kPropertiesName =
    "TitanDB.blob_validation";

bool BlobValidationCollector::Encode(
    const std::unordered_map<uint64_t, std::vector<uint64_t>>&
        blob_entry_indexes,
    std::string* result) {
  PutVarint32(result, static_cast<uint32_t>(blob_entry_indexes.size()));
  for (const auto& item : blob_entry_indexes) {
    PutVarint64(result, item.first);
    PutVarint32(result, static_cast<uint32_t>(item.second.size()));
    for (const auto& entry : item.second) {
      PutVarint64(result, entry);
    }
  }
  return true;
}
bool BlobValidationCollector::Decode(
    Slice* slice,
    std::unordered_map<uint64_t, std::vector<uint64_t>>* blob_entry_indexes) {
  uint32_t num = 0;
  if (!GetVarint32(slice, &num)) {
    return false;
  }
  uint64_t file_number;
  uint32_t size;
  uint64_t index;
  for (uint32_t i = 0; i < num; ++i) {
    if (!GetVarint64(slice, &file_number)) {
      return false;
    }
    if (!GetVarint32(slice, &size)) {
      return false;
    }
    for (uint32_t j = 0; j < size; ++j) {
      if (!GetVarint64(slice, &index)) {
        return false;
      }
      (*blob_entry_indexes)[file_number].push_back(index);
    }
  }
  return true;
}

Status BlobValidationCollector::AddUserKey(const Slice& /* key */,
                                           const Slice& value, EntryType type,
                                           SequenceNumber /* seq */,
                                           uint64_t /* file_size */) {
  if (type != kEntryBlobIndex) {
    return Status::OK();
  }

  BlobIndex index;
  Slice value_copy = value;
  auto s = index.DecodeFrom(&value_copy);
  if (!s.ok()) {
    return s;
  }

  blob_invalid_entries_[index.file_number].push_back(index.blob_handle.index);

  return Status::OK();
}

Status BlobValidationCollector::Finish(UserCollectedProperties* properties) {
  if (blob_invalid_entries_.empty()) {
    return Status::OK();
  }

  std::string res;
  bool ok __attribute__((__unused__)) = Encode(blob_invalid_entries_, &res);
  assert(ok);
  assert(!res.empty());
  properties->emplace(std::make_pair(kPropertiesName, res));
  return Status::OK();
}

}  // namespace titandb
}  // namespace rocksdb
