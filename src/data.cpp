#include "./data.h"

#include "./paw_print.h"


namespace paw_print {


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


}
