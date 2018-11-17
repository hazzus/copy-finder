#include "reader.h"

std::vector<unsigned char> reader::read_byte_data(size_t amount) {
    std::vector<unsigned char> result;
    result.reserve(amount);
    in.read(reinterpret_cast<char*>(result.data()), amount);
    result.resize(in.gcount());
    return result;
}
