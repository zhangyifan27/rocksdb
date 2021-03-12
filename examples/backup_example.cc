// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdio>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "rocksdb/utilities/backupable_db.h"


using namespace ROCKSDB_NAMESPACE;

#if defined(OS_WIN)
std::string kDBPath = "C:\\Windows\\TEMP\\rocksdb_simple_example";
#else
std::string kDBPath = "/tmp/rocksdb_simple_example";
#endif

int main() {
  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  Status s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  // Put key-value
  s = db->Put(WriteOptions(), "key1", "value");
  assert(s.ok());
  std::string value;
  // get value
  s = db->Get(ReadOptions(), "key1", &value);
  assert(s.ok());
  assert(value == "value");

  // backup DB twice
  BackupEngine* backup_engine;
  s = BackupEngine::Open(Env::Default(), BackupableDBOptions("/tmp/rocksdb_backup"), &backup_engine);
  assert(s.ok());
  s = backup_engine->CreateNewBackup(db, true /* flush_before_backup */);
  assert(s.ok());
  //db->Put(WriteOptions(), "key1", "value");;
  s = backup_engine->CreateNewBackup(db, true /* flush_before_backup */);
  assert(s.ok());

  std::vector<BackupInfo> backup_info;
  backup_engine->GetBackupInfo(&backup_info);

  // you can get IDs from backup_info if there are more than two
  s = backup_engine->VerifyBackup(1 /* ID */);
  assert(s.ok());

  // get value
  s = db->Get(ReadOptions(), "key1", &value);
  assert(s.ok());
  assert(value == "value");

  s = backup_engine->VerifyBackup(2 /* ID */);
  assert(s.ok());

  delete backup_engine;

  delete db;

  return 0;
}
