#include <cstdint>

class TLUPayloadDecoder {
    public:
        // TLU payload
        static const std::uint8_t  DCODE_TLU_HEADER = 0xaa;
        static const std::uint8_t  DCODE_TLU_TRAILER = 0xbb;
        static const std::uint16_t DSIZE_TLU_PAYLOAD = 4;

        static bool DecodeEvent(
            std::uint8_t *data, 
            std::uint16_t &trig_num) {
                std::uint8_t *p = data;
                if (((*p & 0xff) != DCODE_TLU_HEADER) || 
                    (*(p + 3) != DCODE_TLU_TRAILER)) {
                    return false;
                }
                // Trigger number MSB
                trig_num = *(p + 1) & 0xff;
                // Trigger number LSB
                trig_num <<= 8;
                trig_num |= *(p + 2) & 0xff;
                // Flags
                return true;
        };
};
