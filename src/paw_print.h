#ifndef PAW_PRINT
#define PAW_PRINT

#include <unordered_map>
#include <vector>
#include <string>

#include "./data.h"


namespace paw_print {


using std::string;
using std::unordered_map;
using std::vector;


class PawPrint {
public:
    class Cursor {
    public:
        Cursor (const PawPrint &paw_print, int idx);

        DataType type ();

        template <class T>
        bool is () { return false; }

        template <class T>
        T get (const T &default_value) {
            if (is<T>() == false)
                return default_value;
            return paw_print_.getData<T>(idx_ + sizeof(DataType));
        }

        const char* get (const char *default_value);
        const char* get (const string &default_value);

    private:
        const PawPrint &paw_print_;
        int idx_;
    };


    inline const vector<unsigned char>& raw_data () {
        return raw_data_;
    }

    PawPrint();
    ~PawPrint();

    void pushInt    (int    value); 
    void pushDouble (double value); 
    void pushString (const char *value); 
    void beginSequence (); 
    void endSequence (); 
    void pushKey (const char *value);
    void beginMap (); 
    void endMap (); 

    DataType type (int idx) const;

    int dataSize (int idx) const;

    Data::StrSizeType getStrSize (int idx) const;
    const char* getStrValue (int idx) const;

    template <class T>
    const T& getData (int idx) const {
        return *((T*)&raw_data_[idx]);
    }

    const vector<int>& getDataIdxsOfSequence (int sequence_idx);
    const vector<int>& getDataIdxsOfMap (int map_idx);

private:
    vector<unsigned char> raw_data_;
    unordered_map<int, vector<int>> data_idxs_of_sequence_map_;
    unordered_map<int, vector<int>> data_idxs_of_map_map_;
    bool is_closed_;


};

template<> bool PawPrint::Cursor::is<int        > ();
template<> bool PawPrint::Cursor::is<double     > ();
template<> bool PawPrint::Cursor::is<const char*> ();
template<> bool PawPrint::Cursor::is<string     > ();

}

#endif
