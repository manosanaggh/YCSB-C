//
//  cs647_db.cc
//  YCSB-C        
//                                                       
//  Created by Manos Anagnostakis on 10/23/22.           
//  Copyright (c) 2022 Manos Anagnostakis <manosanag@csd.uoc.gr>.
//  

#include "cs647_db.h"
#include "cs647/assignment2/ti_api.hpp"

namespace ycsbc {

int CS647DB::Read(const std::string &table, const std::string &key,
         const std::vector<std::string> *fields,
         std::vector<KVPair> &result) {

	std::vector<Tinyblob*> *pv = NULL;

	for(auto field : *fields){
                if((pv = get(field)) != NULL)
                        for(auto x : *pv)
				result.push_back(std::make_pair(key, std::string((char*)x->__io_buffer)));
                else
                        std::cout << "[GET] Key does not exist!" << std::endl;
	}
}

int CS647DB::Scan(const std::string &table, const std::string &key,
           int len, const std::vector<std::string> *fields,
           std::vector<std::vector<KVPair>> &result){}

int CS647DB::Update(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}

int CS647DB::Insert(const std::string &table, const std::string &key,
             std::vector<KVPair> &values){}       

int CS647DB::Delete(const std::string &table, const std::string &key){}

} // namespace ycsbc


