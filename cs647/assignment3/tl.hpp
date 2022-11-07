#ifndef TL
#define TL

#include "ti.hpp"

#define ALIGNMENT 512

class Tinylog{
        private:
        public:
                Tinylog();
                ~Tinylog();
                int fd;
		uint32_t offset;
		std::string wal_buf;
		uint32_t buf_sz;
};

#endif

