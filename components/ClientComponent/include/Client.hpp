#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <stddef.h>

namespace Client
{
    void clientStart(void);
    void clientPublish(const void *message, size_t len);
    // ONLY pass null terminated strings to this function! It uses strlen!
    void clientPublishStr(const char *message);
}

#endif
