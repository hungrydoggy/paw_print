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

		inline int idx () const { return idx_; }
        DataType type () const;

        template <class T>
        bool is () const { return false; }

        bool isSequence () const;
        bool isMap () const;

        template <class T>
        T get (T default_value) const {
            if (is<T>() == false)
                return default_value;
            return paw_print_.getData<T>(idx_ + sizeof(DataType));
        }

        const char* get (const string &default_value) const;

        Cursor operator[] (int idx) const;
        Cursor operator[] (const char *key) const;
        Cursor operator[] (const string &key) const;

        int size () const;

    private:
        const PawPrint &paw_print_;
        int idx_;
    };


    inline const vector<unsigned char>& raw_data () const {
        return raw_data_;
    }

    PawPrint();
    ~PawPrint();

    DataType type (int idx) const;

    int dataSize (int idx) const;


    // write
    void pushInt    (int    value); 
    void pushDouble (double value); 
    void pushString (const char *value); 
    void beginSequence (); 
    void endSequence (); 
    void pushKey (const char *value);
    void beginMap (); 
    void endMap (); 


    // read
    Cursor root () const;

    Data::StrSizeType getStrSize (int idx) const;
    const char* getStrValue (int idx) const;

    int getKeyRawIdxOfPair   (int pair_idx) const;
    int getValueRawIdxOfPair (int pair_idx) const;

    template <class T>
    const T& getData (int idx) const {
        return *((T*)&raw_data_[idx]);
    }

    const vector<int>& getDataIdxsOfSequence (int sequence_idx) const;
    const vector<int>& getDataIdxsOfMap (int map_idx) const;

    int findRawIdxOfValue (
            const vector<int> &map_datas,
            int first,
            int last,
            const char *key) const;

private:
    vector<unsigned char> raw_data_;
    mutable unordered_map<int, vector<int>> data_idxs_of_sequence_map_;
    mutable unordered_map<int, vector<int>> data_idxs_of_map_map_;
    bool is_closed_;


};

template<> bool PawPrint::Cursor::is<int        > () const;
template<> bool PawPrint::Cursor::is<double     > () const;
template<> bool PawPrint::Cursor::is<const char*> () const;
template<> bool PawPrint::Cursor::is<string     > () const;

template<> const char* PawPrint::Cursor::get<const char*> (const char *default_value) const;

}

#endif
