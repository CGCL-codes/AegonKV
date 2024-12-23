#ifndef YCSB_C_TITANDB_DB_H_
#define YCSB_C_TITANDB_DB_H_

#include <titan/db.h>
#include <titan/options.h>

#include <mutex>
#include <string>

#include "core/db.h"
#include "core/properties.h"

namespace ycsbc {

class DiffKV : public DB {
 public:
  DiffKV() {}
  ~DiffKV() {}

  void Init() override;

  void Cleanup() override;

  void Statistics() override;

  void OnTransactionFinished() override;

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields,
              std::vector<Field> &result) override {
    return (this->*(method_read_))(table, key, fields, result);
  }

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields,
              std::vector<std::vector<Field>> &result) {
    return (this->*(method_scan_))(table, key, len, fields, result);
  }

  Status Update(const std::string &table, const std::string &key,
                std::vector<Field> &values) {
    return (this->*(method_update_))(table, key, values);
  }

  Status Insert(const std::string &table, const std::string &key,
                std::vector<Field> &values) {
    return (this->*(method_insert_))(table, key, values);
  }

  Status Delete(const std::string &table, const std::string &key) {
    return (this->*(method_delete_))(table, key);
  }

 private:
  enum TitanFormat {
    kSingleRow,
  };
  TitanFormat format_;

  void GetOptions(const utils::Properties &props,
                  rocksdb::titandb::TitanOptions *opt);
  static void SerializeRow(const std::vector<Field> &values, std::string &data);
  static void DeserializeRowFilter(std::vector<Field> &values, const char *p,
                                   const char *lim,
                                   const std::vector<std::string> &fields);
  static void DeserializeRowFilter(std::vector<Field> &values,
                                   const std::string &data,
                                   const std::vector<std::string> &fields);
  static void DeserializeRow(std::vector<Field> &values, const char *p,
                             const char *lim);
  static void DeserializeRow(std::vector<Field> &values,
                             const std::string &data);

  Status ReadSingle(const std::string &table, const std::string &key,
                    const std::vector<std::string> *fields,
                    std::vector<Field> &result);
  Status ScanSingle(const std::string &table, const std::string &key, int len,
                    const std::vector<std::string> *fields,
                    std::vector<std::vector<Field>> &result);
  Status UpdateSingle(const std::string &table, const std::string &key,
                      std::vector<Field> &values);
  Status MergeSingle(const std::string &table, const std::string &key,
                     std::vector<Field> &values);
  Status InsertSingle(const std::string &table, const std::string &key,
                      std::vector<Field> &values);
  Status DeleteSingle(const std::string &table, const std::string &key);

  Status (DiffKV::*method_read_)(const std::string &, const std::string &,
                                    const std::vector<std::string> *,
                                    std::vector<Field> &);
  Status (DiffKV::*method_scan_)(const std::string &, const std::string &,
                                    int, const std::vector<std::string> *,
                                    std::vector<std::vector<Field>> &);
  Status (DiffKV::*method_update_)(const std::string &, const std::string &,
                                      std::vector<Field> &);
  Status (DiffKV::*method_insert_)(const std::string &, const std::string &,
                                      std::vector<Field> &);
  Status (DiffKV::*method_delete_)(const std::string &, const std::string &);

  int fieldcount_;
  rocksdb::WriteOptions write_options_;
  rocksdb::ReadOptions read_options_;

  static rocksdb::titandb::TitanDB *db_;
  static int ref_cnt_;
  static std::mutex mu_;
};

DB *NewDiffKV();

}  // namespace ycsbc

#endif  // YCSB_C_TITANDB_DB_H_
