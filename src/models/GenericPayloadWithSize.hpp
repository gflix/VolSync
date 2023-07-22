#ifndef MODELS_GENERICPAYLOADWITHSIZE_HPP_
#define MODELS_GENERICPAYLOADWITHSIZE_HPP_

#include <cstdint>
#include <ostream>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) GenericPayloadWithSize
{
    explicit GenericPayloadWithSize(
        uint64_t size = 0);
    ~GenericPayloadWithSize();

    uint64_t size;

    ByteArray toByteArray(void) const;
    static GenericPayloadWithSize fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const GenericPayloadWithSize& item);

} /* namespace VolSync */

#endif /* MODELS_GENERICPAYLOADWITHSIZE_HPP_ */
