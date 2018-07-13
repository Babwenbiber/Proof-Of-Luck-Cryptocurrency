#include <sodiumpp/base64.h>
std::string base64_encode(unsigned char const* buf, unsigned int bufLen) {
    size_t ret_size = bufLen+2;

    ret_size = 4*ret_size/3;

    std::string ret;
    ret.reserve(ret_size);

    for (unsigned int i=0; i<ret_size/4; ++i)
    {
        size_t index = i*3;
        unsigned char b3[3];
        b3[0] = buf[index+0];
        b3[1] = buf[index+1];
        b3[2] = buf[index+2];

        ret.push_back(to_base64[ ((b3[0] & 0xfc) >> 2) ]);
        ret.push_back(to_base64[ ((b3[0] & 0x03) << 4) + ((b3[1] & 0xf0) >> 4) ]);
        ret.push_back(to_base64[ ((b3[1] & 0x0f) << 2) + ((b3[2] & 0xc0) >> 6) ]);
        ret.push_back(to_base64[ ((b3[2] & 0x3f)) ]);
    }

    return ret;
}

std::vector<unsigned char> base64_decode(std::string encoded_string) {
    size_t encoded_size = encoded_string.size();
    std::vector<unsigned char> ret;
    ret.reserve(3*encoded_size/4);

    for (size_t i=0; i<encoded_size; i += 4)
    {
        unsigned char b4[4];
        b4[0] = from_base64[encoded_string[i+0]];
        b4[1] = from_base64[encoded_string[i+1]];
        b4[2] = from_base64[encoded_string[i+2]];
        b4[3] = from_base64[encoded_string[i+3]];

        unsigned char b3[3];
        b3[0] = ((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4);
        b3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2);
        b3[2] = ((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f));

        ret.push_back(b3[0]);
        ret.push_back(b3[1]);
        ret.push_back(b3[2]);
    }

    return ret;
}
