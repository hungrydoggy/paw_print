#include "./paw_print.h"


namespace paw_print {


PawPrint::Cursor::Cursor (const PawPrint &paw_print, int idx)
:paw_print_(paw_print),
 idx_(idx) {
}

DataType PawPrint::Cursor::type () const {
    if (idx_ < 0)
        return Data::TYPE_NONE;

    return paw_print_.type(idx_);
}

template<> bool PawPrint::Cursor::is<int        > () const { return type() == Data::TYPE_INT;    }
template<> bool PawPrint::Cursor::is<double     > () const { return type() == Data::TYPE_DOUBLE; }
template<> bool PawPrint::Cursor::is<const char*> () const { return type() == Data::TYPE_STRING; }
template<> bool PawPrint::Cursor::is<string     > () const { return type() == Data::TYPE_STRING; }

bool PawPrint::Cursor::isSequence () const { return type() == Data::TYPE_SEQUENCE; }
bool PawPrint::Cursor::isMap      () const { return type() == Data::TYPE_MAP;      }

const char* PawPrint::Cursor::get (const char *default_value) const {
    if (is<const char*>() == false)
        return default_value;
    return paw_print_.getStrValue(idx_);
}

const char* PawPrint::Cursor::get (const string &default_value) const {
    if (is<const char*>() == false)
        return default_value.c_str();
    return paw_print_.getStrValue(idx_);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (int idx) const {
    if (isSequence() == false)
        return Cursor(paw_print_, -1);

    auto &data_idxs = paw_print_.getDataIdxsOfSequence(idx_);
    if (idx < 0 || idx >= data_idxs.size())
        return Cursor(paw_print_, -1);

    return Cursor(paw_print_, data_idxs[idx]);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const char *key) const {
    if (isMap() == false)
        return Cursor(paw_print_, -1);

    auto &data_idxs = paw_print_.getDataIdxsOfSequence(idx_);
    int idx = paw_print_.findRawIdxOfValue(data_idxs, 0, data_idxs.size() - 1, key);
    if (idx < 0)
        return Cursor(paw_print_, -1);

    auto pair_idx = data_idxs[idx];
    auto value_idx = paw_print_.getValueRawIdxOfPair(pair_idx);
    return Cursor(paw_print_, data_idxs[value_idx]);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const string &key) const {
    return operator [] (key.c_str());
}


}
