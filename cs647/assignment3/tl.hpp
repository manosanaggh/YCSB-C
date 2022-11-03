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
		std::vector<std::pair<std::string, std::string>> wal_buf;
};

#endif

