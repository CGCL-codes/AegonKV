//
//  client.h
//  YCSB-cpp
//
//  Copyright (c) 2020 Youngjae Lee <ls4154.lee@gmail.com>.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_CLIENT_H_
#define YCSB_C_CLIENT_H_

#include <string>

#include "core_workload.h"
#include "countdown_latch.h"
#include "db.h"
#include "utils.h"

namespace ycsbc {

inline int ClientThread(ycsbc::DB *db, ycsbc::CoreWorkload *wl,
                        const int num_ops, bool is_loading, bool init_db,
                        bool cleanup_db, CountDownLatch *latch) {
  try {
    if (init_db) {
      db->Init();
    }

    int ops = 0;
    bool op_OK;
    //    if (!is_loading) {
    //      wl->PrepareRandomInsert(num_ops);
    //    }
    for (int i = 0; i < num_ops; ++i) {
      if (is_loading) {
        op_OK = wl->DoInsert(*db);
      } else {
        op_OK = wl->DoTransaction(*db);
        //        op_OK = wl->DoRandomInsert(*db);
      }
      if (op_OK) {
        ops++;
      }
    }

    if (cleanup_db) {
      db->Cleanup();
    }

    latch->CountDown();
    return ops;
  } catch (const utils::Exception &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    exit(1);
  }
}

}  // namespace ycsbc

#endif  // YCSB_C_CLIENT_H_
