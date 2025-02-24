#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <stddef.h>
#include <cstdint>

namespace Client
{
    void clientStart(void);
    void clientPublish(char* data_type, void* value);
    // ONLY pass null terminated strings to this function! It uses strlen!
    void clientPublishStr(const char *message);
}

#endif
