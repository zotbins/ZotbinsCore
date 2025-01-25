#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <stdint.h>

namespace Client
{
    void clientStart(void);
    void clientPublish(char *message, size_t len);
}

#endif
