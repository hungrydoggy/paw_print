#include "./paw_print.h"

#include <algorithm>
#include <iostream>


namespace paw_print {

PawPrint::PawPrint ()
:is_closed_(false) {
}

PawPrint::~PawPrint () {
}

const vector<Data*>& PawPrint::getDatasOfSequence (SequenceData *data) {
    if (datas_of_sequence_map_.find(data) != datas_of_sequence_map_.end())
        return datas_of_sequence_map_[data];


    auto &result = datas_of_sequence_map_[data];
    auto p = ((unsigned char *)data) + sizeof(DataType);
    auto dp = (Data*)p;
    while (dp->type() != Data::TYPE_SEQUENCE_END) {
        result.push_back(dp);

        p += dp->dataSize(this);
        dp = (Data*)p;
    }
    
    return result;
}

Data* PawPrint::getDataOfSequence (SequenceData *data, int idx) {
    auto &datas = getDatasOfSequence(data);
    if (idx < 0 || idx >= datas.size())
        return 0;

    return datas[idx];
}

static bool kvsort_func (KeyValuePairData *a, KeyValuePairData *b) {
    auto ap = a->key()->value();
    auto bp = b->key()->value();

    return strcmp(ap, bp) < 0;
}

const vector<KeyValuePairData*>& PawPrint::getDatasOfMap (MapData *data) {
    if (datas_of_map_map_.find(data) != datas_of_map_map_.end())
        return datas_of_map_map_[data];

    // make datas
    auto &result = datas_of_map_map_[data];
    auto p = ((unsigned char *)data) + sizeof(DataType);
    auto dp = (Data*)p;
    std::cout << "dp : " << (int)dp->type() << std::endl;
    exit(0);
    while (dp->type() != Data::TYPE_MAP_END) {
        std::cout << "6";
        result.push_back((KeyValuePairData*)dp);

        std::cout << "7";
        p += dp->dataSize(this);
        std::cout << "8";
        dp = (Data*)p;

        std::cout << ((KeyValuePairData*)dp)->key()->value()
                << ((StringData*)((KeyValuePairData*)dp)->value())->value() << std::endl;
    }

    std::sort(result.begin(), result.end(), kvsort_func);
    
    return result;
}

static Data* _searchByBinary (
        const vector<KeyValuePairData*> &map_datas, int first, int last, const char *key) {

    if (first > last)
        return 0;

    int mid = (first + last) / 2;
    auto mid_data = map_datas[mid];
    auto mid_value = mid_data->key()->value();

    auto cmp_res = strcmp(key, mid_value);
    if (cmp_res < 0)
        return _searchByBinary(map_datas, first, mid - 1, key);
    else if (cmp_res > 0)
        return _searchByBinary(map_datas, mid + 1, last, key);
    else
        return mid_data->value();
}

Data* PawPrint::getDataOfMap (MapData *data, const char *key) {
    auto &map_datas = getDatasOfMap(data);

    return _searchByBinary(map_datas, 0, map_datas.size() - 1, key);
}

void PawPrint::pushInt (int value) {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType) + sizeof(int));
    *((DataType*)&raw_data_[old_size                   ]) = Data::TYPE_INT;
    *((int*     )&raw_data_[old_size + sizeof(DataType)]) = value;
}

void PawPrint::pushDouble (double value) {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType) + sizeof(double));
    *((DataType*)&raw_data_[old_size                   ]) = Data::TYPE_DOUBLE;
    *((double*  )&raw_data_[old_size + sizeof(DataType)]) = value;
}

void PawPrint::pushString (const char *value) {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    int str_count = strlen(value) + 1;
    raw_data_.resize(
            old_size
            + sizeof(DataType)
            + sizeof(StringData::StrSizeType)
            + sizeof(const char) * str_count);
    *((DataType*               )&raw_data_[old_size                   ]) = Data::TYPE_STRING;
    *((StringData::StrSizeType*)&raw_data_[old_size + sizeof(DataType)]) = str_count;
    auto p = (const char*)&raw_data_[
            old_size + sizeof(DataType) + sizeof(StringData::StrSizeType)];
    memcpy((void*)p, (void*)value, sizeof(const char) * str_count);
}

void PawPrint::beginSequence () {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType));
    *((DataType*)&raw_data_[old_size]) = Data::TYPE_SEQUENCE_START;
}

void PawPrint::endSequence () {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType));
    *((DataType*)&raw_data_[old_size]) = Data::TYPE_SEQUENCE_END;
}

void PawPrint::pushKey (const char *value) {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType));
    *((DataType*)&raw_data_[old_size]) = Data::TYPE_KEY_VALUE_PAIR;

    pushString(value);
}

void PawPrint::beginMap () {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType));
    *((DataType*)&raw_data_[old_size]) = Data::TYPE_MAP_START;
}

void PawPrint::endMap () {
    if (is_closed_ == true)
        return;

    int old_size = raw_data_.size();
    raw_data_.resize(old_size + sizeof(DataType));
    *((DataType*)&raw_data_[old_size]) = Data::TYPE_MAP_END;
}


}
