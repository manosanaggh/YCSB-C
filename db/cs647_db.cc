//
//  cs647_db.cc
//  YCSB-C        
//                                                       
//  Created by Manos Anagnostakis on 10/23/22.           
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//  

#include "cs647_db.h"

namespace ycsbc {

int CS647DB::Read(const std::string &table, const std::string &key,
         const std::vector<std::string> *fields,
         std::vector<KVPair> &result) {   }

int CS647DB::Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){}

int CS647DB::Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}

int CS647DB::Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}       

int CS647DB::Delete(const std::string &table, const std::string &key){}

} // namespace ycsbc


