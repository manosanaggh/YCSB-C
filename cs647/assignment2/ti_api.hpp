#ifndef TI_API
#define TI_API

#include "ti.hpp"
#include "tb.hpp"

int put(void *args); //Return 0 on success -1 on failure.

std::vector<Tinyblob*> *get(std::string key); //Return a buffer filled with the value if key is found, null if key is not found.

//std::vector<Tinyblob*> *get(void *args); //Return a buffer filled with the value if key is found, null if key is not found.

int erase(void *args); // Return 0 on success -1 on failure.

std::ifstream scan_init(void); // Initialize a scanner, return a pointer to a scanner handle on success NULL on failure. 

int get_next(std::ifstream *scanner); // Iterate to the next KV, return 0 if handle moved forward or -1 if it reached the end. 

std::string get_scan_key(std::string current_pair); // Get a pointer to the current key for this scanner.

std::vector<Tinyblob*> *get_scan_value(std::string current_pair); // Get a pointer to the current value for this scanner.

int close_scanner(std::ifstream *scanner); // Release the resources for the scanner.

void persist(char *location); // Store all the key values on the location path.

void recover(char *location); // Restore all the key values stored on the location path.

#endif
