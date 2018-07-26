#include "./paw_print.h"

#include <algorithm>


namespace paw_print {

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
    while (dp->type() != Data::TYPE_MAP_END) {
        result.push_back((KeyValuePairData*)dp);

        p += dp->dataSize(this);
        dp = (Data*)p;
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

const Data* PawPrint::getDataOfMap (MapData *data, const char *key) {
    auto &map_datas = getDatasOfMap(data);

    return _searchByBinary(map_datas, 0, map_datas.size() - 1, key);
}

}
