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
    class Cursor {
    private:
        int idx_;
    };

    PawPrint();
    ~PawPrint();

    const vector<Data*>& getDatasOfSequence (SequenceData *data);
    Data* getDataOfSequence (SequenceData *data, int idx);

    const vector<KeyValuePairData*>& getDatasOfMap (MapData *data);
    Data* getDataOfMap (MapData *data, const char *key);

    void pushInt    (int    value); 
    void pushDouble (double value); 
    void pushString (const char *value); 
    void beginSequence (); 
    void endSequence (); 
    void pushKey (const char *value);
    void beginMap (); 
    void endMap (); 

    inline const vector<unsigned char>& raw_data () {
        return raw_data_;
    }

    inline Data* root () {
        if (raw_data_.size() <= 0)
            return 0;

        return (Data*)&raw_data_[0];
    }

private:
    vector<unsigned char> raw_data_;
    unordered_map<SequenceData*, vector<Data*>> datas_of_sequence_map_;
    unordered_map<MapData*, vector<KeyValuePairData*>> datas_of_map_map_;
    bool is_closed_;
};

}

#endif
