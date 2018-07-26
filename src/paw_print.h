#ifndef PAW_PRINT
#define PAW_PRINT

#include <unordered_map>
#include <vector>

#include "./data.h"


namespace paw_print {


using std::unordered_map;
using std::vector;


class PawPrint {
public:
    const vector<Data*>& getDatasOfSequence (SequenceData *data);

    const vector<KeyValuePairData*>& getDatasOfMap (MapData *data);
    const Data* getDataOfMap (MapData *data, const char *key);

private:
    unordered_map<SequenceData*, vector<Data*>> datas_of_sequence_map_;
    unordered_map<MapData*, vector<KeyValuePairData*>> datas_of_map_map_;
};

}

#endif
