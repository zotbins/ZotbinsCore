#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <stddef.h>
#include <cstdint>

namespace Client
{
    void clientStart(void);
    void clientPublish(char* data_type, void* value);
    void clientPublish(const void *message, size_t len);

    // ONLY pass null terminated strings to this function! It uses strlen!
    void clientPublishStr(const char *message);
    // void clientInitPhotoSend(int photoID, int totalCount); 
    // void clientPhotoSend(int photoID, int imageData, char* image); 
}

#endif
