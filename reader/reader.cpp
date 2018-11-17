#include "reader.h"

std::vector<char> reader::read_byte_data(size_t amount) {
    std::vector<char> result;
    result.resize(amount);
    in.read(result.data(), amount);
    result.resize(in.gcount());
    return result;
}
