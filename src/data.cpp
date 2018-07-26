#include "./data.h"

#include <iostream>

#include "./paw_print.h"


namespace paw_print {


Data::Data (unsigned char *start)
:start_(start) {
}

int IntData::dataSize (PawPrint *paw_print) {
    return sizeof(DataType) + sizeof(int);
}

int DoubleData::dataSize (PawPrint *paw_print) {
    return sizeof(DataType) + sizeof(double);
}

int StringData::dataSize (PawPrint *paw_print) {
    return sizeof(DataType) + sizeof(StrSizeType) + strSize();
}

int SequenceData::dataSize (PawPrint *paw_print) {
    auto &datas = paw_print->getDatasOfSequence(this);
    int result = sizeof(DataType);
    for (auto &d : datas)
        result += d->dataSize(paw_print);

    return result;
}

Data* SequenceData::get (PawPrint *paw_print, int idx) {
    return paw_print->getDataOfSequence(this, idx);
}

int KeyValuePairData::dataSize (PawPrint *paw_print) {
    return sizeof(DataType) + key()->dataSize(paw_print) + value()->dataSize(paw_print);
}

int MapData::dataSize (PawPrint *paw_print) {
    auto &datas = paw_print->getDatasOfMap(this);
    int result = sizeof(DataType);
    for (auto &d : datas)
        result += d->dataSize(paw_print);

    return result;
}

Data* MapData::get (PawPrint *paw_print, const char *key) {
    std::cout << "kkkkkkk" << std::endl;
    return paw_print->getDataOfMap(this, key);
}


}
