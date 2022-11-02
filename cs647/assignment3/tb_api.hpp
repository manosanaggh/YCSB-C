#ifndef TB_API
#define TB_API

#include "tb.hpp"

int tb_allocate_blob(); // On success it returns the index/id of the allocated blob in the store. On failure it returns -1; When using files you can use a naming scheme “Bxxx” to name each blob file.
void tb_free_blob(int); // Given a valid index from a successful allocate_blob call, it frees the blob.
int tb_write_blob(int, void *data); // Perform a synchronous write (using issue_write() or otherwise) from the data buffer. Return success or failure.
int tb_read_blob(void *args); // Perform a synchronous read (using issue_read() or otherwise) to the data buffer. Return success or failure.
void tb_flush(); // Flush all data and metadata to the disk.  NOTE! Be careful when you try to persist structs, you need to add __attribute__((__packed__)); otherwise the compiler may add padding for alignment purposes. Read more in the following link https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Type-Attributes.html
void tb_init(char *location); // Given a directory or a file, it reads the library state in memory and is able to perform I/O to the store. 
void tb_shutdown(void); // Clean shutdown of the store; Flush all data and metadata so that it is possible to start from the same files/device.

#endif
