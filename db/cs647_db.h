//
//  cs647_db.h
//  YCSB-C
//
//  Created by Manos Anagnostakis on 10/23/22.
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//

#ifndef YCSB_C_CS647_DB_H_
#define YCSB_C_CS647_DB_H_

#include "core/db.h"
#include "core/properties.h"
#include <string>
#include <vector>

namespace ycsbc {             

class CS647DB : public DB {
 public:
  //Read a single record
  int Read(const std::string &table, const std::string &key,
           const std::vector<std::string> *fields,
           std::vector<KVPair> &result);

  //Perform a range scan
  int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result);

  //Update a single record
  int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);

  //Insert a single record
  int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values);

  //Delete a single record
  int Delete(const std::string &table, const std::string &key);
};

} // ycsbc

#endif // YCSB_C_CS647_DB_H_


