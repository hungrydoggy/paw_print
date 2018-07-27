#include "./paw_print.h"

#include <algorithm>
#include <iostream>


namespace paw_print {

    

PawPrint::PawPrint ()
:is_closed_(false) {
}

PawPrint::~PawPrint () {
}

DataType PawPrint::type (int idx) const {
    return getData<DataType>(idx);
}

Data::StrSizeType PawPrint::getStrSize (int idx) const {
    return getData<Data::StrSizeType>(idx + sizeof(DataType));
}

const char* PawPrint::getStrValue (int idx) const {
    return (const char*)&raw_data_[idx + sizeof(DataType) + sizeof(Data::StrSizeType)];
}

int PawPrint::dataSize (int idx) const {
    int result = sizeof(DataType);

    auto t = type(idx);
    switch (t) {
        case Data::TYPE_INT   : result += sizeof(int   ); break;
        case Data::TYPE_DOUBLE: result += sizeof(double); break;

        case Data::TYPE_STRING:
            result += sizeof(Data::StrSizeType) + sizeof(const char) * getStrSize(idx);
            break;

        case Data::TYPE_SEQUENCE_START: break;
        case Data::TYPE_SEQUENCE_END: break;

        case Data::TYPE_MAP_START: break;
        case Data::TYPE_MAP_END: break;

        case Data::TYPE_KEY_VALUE_PAIR:
            result += dataSize(idx + result); // key size
            result += dataSize(idx + result); // value size
            break;

        default: break;
    }

    return result;
}

const vector<int>& PawPrint::getDataIdxsOfSequence (int sequence_idx) {
    if (data_idxs_of_sequence_map_.find(sequence_idx) != data_idxs_of_sequence_map_.end())
        return data_idxs_of_sequence_map_[sequence_idx];

    auto &result = data_idxs_of_sequence_map_[sequence_idx];
    auto idx = sequence_idx + sizeof(DataType);
    while (type(idx) != Data::TYPE_SEQUENCE_END) {
        result.push_back(idx);

        idx += dataSize(idx);
    }
    
    return result;
}

class SortFuncForKey {
public:
    SortFuncForKey (const PawPrint &paw_print)
    :paw_print_(paw_print) {
    }

    bool operator () (int a_idx, int b_idx) {
        return strcmp(paw_print_.getStrValue(a_idx), paw_print_.getStrValue(b_idx)) < 0;
    }

private:
    const PawPrint &paw_print_;
};

const vector<int>& PawPrint::getDataIdxsOfMap (int map_idx) {
    if (data_idxs_of_map_map_.find(map_idx) != data_idxs_of_map_map_.end())
        return data_idxs_of_map_map_[map_idx];

    // make datas
    auto &result = data_idxs_of_map_map_[map_idx];
    auto idx = map_idx + sizeof(DataType);
    while (type(idx) != Data::TYPE_MAP_END) {
        result.push_back(idx);

        idx += dataSize(idx);
    }

    std::sort(result.begin(), result.end(), SortFuncForKey(*this));
    
    return result;
}

static int _searchByBinary (
        const PawPrint &paw_print,
        const vector<int> &map_datas,
        int first,
        int last,
        const char *key) {

    if (first > last)
        return 0;

    int mid = (first + last) / 2;
    auto mid_data_idx = map_datas[mid];
    auto mid_key = paw_print.getStrValue(mid_data_idx);

    auto cmp_res = strcmp(key, mid_key);
    if (cmp_res < 0)
        return _searchByBinary(paw_print, map_datas, first, mid - 1, key);
    else if (cmp_res > 0)
        return _searchByBinary(paw_print, map_datas, mid + 1, last, key);
    else
        return mid_data_idx;
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
            + sizeof(Data::StrSizeType)
            + sizeof(const char) * str_count);
    *((DataType*               )&raw_data_[old_size                   ]) = Data::TYPE_STRING;
    *((Data::StrSizeType*)&raw_data_[old_size + sizeof(DataType)]) = str_count;
    auto p = (const char*)&raw_data_[
            old_size + sizeof(DataType) + sizeof(Data::StrSizeType)];
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
