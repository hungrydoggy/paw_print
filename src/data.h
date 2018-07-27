#ifndef PAW_PRINT_DATA
#define PAW_PRINT_DATA

namespace paw_print {

using DataType = unsigned char;


class Data {
public:
    using StrSizeType = unsigned short;

    static const DataType TYPE_NONE           = 0;
    static const DataType TYPE_INT            = 1;
    static const DataType TYPE_DOUBLE         = 2;
    static const DataType TYPE_STRING         = 3;

    static const DataType TYPE_SEQUENCE       = 4;
    static const DataType TYPE_SEQUENCE_START = 4;
    static const DataType TYPE_SEQUENCE_END   = 5;

    static const DataType TYPE_MAP            = 6;
    static const DataType TYPE_MAP_START      = 6;
    static const DataType TYPE_MAP_END        = 7;

    static const DataType TYPE_KEY_VALUE_PAIR = 8;
};

}

#endif
