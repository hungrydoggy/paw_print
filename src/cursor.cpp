#include "./paw_print.h"

#include <iomanip>
#include <iostream>
#include <sstream>


namespace paw_print {


using std::cout;
using std::endl;
using std::stringstream;


PawPrint::Cursor::Cursor (const PawPrint &paw_print, int idx)
:paw_print_(paw_print),
 idx_(idx) {
}

PawPrint::Cursor::Cursor (const Cursor &cursor)
:paw_print_(cursor.paw_print_),
 idx_(cursor.idx_) {
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

template<>
double PawPrint::Cursor::get<double> (double default_value) const {
    if      (is<double>() == true)
        return paw_print_.getData<double>(idx_ + sizeof(DataType));
    else if (is<int   >() == true)
        return paw_print_.getData<int   >(idx_ + sizeof(DataType));
    else
        return default_value;
}

template<>
const char* PawPrint::Cursor::get<const char*> (const char *default_value) const {
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
    if (isSequence() == true) {
        auto &data_idxs = paw_print_.getDataIdxsOfSequence(idx_);
        if (idx < 0 || idx >= data_idxs.size())
            return Cursor(paw_print_, -1);

        return Cursor(paw_print_, data_idxs[idx]);
    }else if (isMap() == true) {
        auto &data_idxs = paw_print_.getDataIdxsOfMap(idx_);
        int pair_idx = data_idxs[idx];
        if (pair_idx < 0)
            return Cursor(paw_print_, -1);

        auto value_idx = paw_print_.getValueRawIdxOfPair(pair_idx);
        return Cursor(paw_print_, value_idx);
    }

    return Cursor(paw_print_, -1);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const char *key) const {
    if (isMap() == false)
        return Cursor(paw_print_, -1);

    auto &data_idxs = paw_print_.getDataIdxsOfMap(idx_);
    int pair_idx = paw_print_.findRawIdxOfValue(data_idxs, 0, data_idxs.size() - 1, key);
    if (pair_idx < 0)
        return Cursor(paw_print_, -1);

    auto value_idx = paw_print_.getValueRawIdxOfPair(pair_idx);
    return Cursor(paw_print_, value_idx);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const string &key) const {
    return operator [] (key.c_str());
}

const char* PawPrint::Cursor::getKey (int idx) const {
    if (isMap() == false)
        return null;

    auto &data_idxs = paw_print_.getDataIdxsOfMap(idx_);
    int pair_idx = data_idxs[idx];
    if (pair_idx < 0)
        return null;

    auto key_idx = paw_print_.getKeyRawIdxOfPair(pair_idx);
    return paw_print_.getStrValue(key_idx);
}

int PawPrint::Cursor::size () const {
    if (isSequence() == true)
        return paw_print_.getDataIdxsOfSequence(idx_).size();
    else if (isMap() == true)
        return paw_print_.getDataIdxsOfMap(idx_).size();
    else
        return 1;
}

string PawPrint::Cursor::toString (int indent, int indent_inc, bool ignore_indent) const {

    stringstream ss;

    if (ignore_indent == false) {
        for (int i=0; i<indent; ++i)
            ss << " ";
    }

    switch (type()) {
        case PawPrint::Data::TYPE_INT:
            ss << get(0) << endl;
            break;
        case PawPrint::Data::TYPE_DOUBLE:
            ss << std::fixed << std::setprecision(8) << get(0.0) << endl;
            break;
        case PawPrint::Data::TYPE_STRING:
            ss << "\"" << get("") << "\"" << endl;
            break;
        case PawPrint::Data::TYPE_SEQUENCE:
			for (int i = 0; i < size(); ++i) {
				if (i != 0) {
					for (int i = 0; i<indent; ++i)
						ss << " ";
				}
				ss << "- " << (*this)[i].toString(indent + indent_inc, indent_inc, true);
			}
            break;
        case PawPrint::Data::TYPE_MAP:
            for (int i=0; i<size(); ++i) {
				if (i != 0) {
					for (int i = 0; i<indent; ++i)
						ss << " ";
				}
                ss << getKey(i) << " :" << endl;
                ss << (*this)[i].toString(indent + indent_inc, indent_inc);
            }
            break;
        default:
            // TODO err
            cout << "err: cannot cursor convert to string type \'"
                    << type() << "\'" << endl;
            return "";
    }

    return ss.str();
}

}
