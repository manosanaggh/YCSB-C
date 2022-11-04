#ifndef TL_API
#define TL_API

#include "tl.hpp"

#define INTERVAL 2000

void append(std::string key, std::string value); //Appends key, value in the log

void checkpoint_metadata(); // checkpoints all metadata of the kv store

int truncate(); //Truncates the existing log, return 0 on success -1 on failure

Tinyindex *replay(); //Returns a tinyindex if there are valid entries in the write-ahead log

#endif
