//  YCSB-C       
//                                                       
//  Created by Manos Anagnostakis on 10/23/22.           
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//  

#include "cs647_db.h"

namespace ycsbc {

int RedisDB::Read(const string &table, const string &key,
         const vector<string> *fields,
         vector<KVPair> &result) {   }

int Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){}

int Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}

int Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}       

int Delete(const std::string &table, const std::string &key){}

};


