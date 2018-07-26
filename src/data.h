#ifndef PAW_PRINT_DATA
#define PAW_PRINT_DATA

namespace paw_print {

class PawPrint;

using DataType = unsigned char;


class Data {
public:
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

    static inline DataType type (unsigned char *p) {
        return *((DataType*)p);
    }

    Data (unsigned char *start);
    virtual ~Data () {}

    inline DataType type () {
        return type(start_);
    }

    virtual int dataSize (PawPrint *paw_print) = 0;

protected:
    unsigned char *start_;

    virtual void _onInit () {}
};

class IntData : public Data {
public:
    inline int value () {
        return *((int*)(start_ + sizeof(DataType)));
    }

    int dataSize (PawPrint *paw_print) override;
};

class DoubleData : public Data {
public:
    inline double value () {
        return *((double*)(start_ + sizeof(DataType)));
    }

    int dataSize (PawPrint *paw_print) override;
};

class StringData : public Data {
public:
    using StrSizeType = unsigned short;
    inline StrSizeType strSize () {
        return *((StrSizeType*)(start_ + sizeof(DataType)));
    }

    inline const char* value () {
        return (const char*)(start_ + sizeof(DataType) + sizeof(StrSizeType));
    }

    int dataSize (PawPrint *paw_print) override;
};

class SequenceData : public Data {
public:
    int dataSize (PawPrint *paw_print) override;

    Data* get (PawPrint *paw_print, int idx);
};

class KeyValuePairData : public Data {
public:
    inline StringData* key () {
        return (StringData*)(start_ + sizeof(DataType));
    }

    inline Data* value () {
        auto k = key();
        return (Data*)(((unsigned char*)k) + k->dataSize(0));
    }

    int dataSize (PawPrint *paw_print) override;
};

class MapData : public Data {
public:    
    int dataSize (PawPrint *paw_print) override;

    Data* get (PawPrint *paw_print, const char *key);
};


}

#endif
