#include "./paw_print.h"


namespace paw_print {


PawPrint::Cursor::Cursor (const PawPrint &paw_print, int idx)
:paw_print_(paw_print),
 idx_(idx) {
}

DataType PawPrint::Cursor::type () {
    return paw_print_.type(idx_);
}

template<> bool PawPrint::Cursor::is<int        > () { return type() == Data::TYPE_INT;    }
template<> bool PawPrint::Cursor::is<double     > () { return type() == Data::TYPE_DOUBLE; }
template<> bool PawPrint::Cursor::is<const char*> () { return type() == Data::TYPE_STRING; }
template<> bool PawPrint::Cursor::is<string     > () { return type() == Data::TYPE_STRING; }

const char* PawPrint::Cursor::get (const char *default_value) {
    if (is<const char*>() == false)
        return default_value;
    return paw_print_.getStrValue(idx_);
}

const char* PawPrint::Cursor::get (const string &default_value) {
    if (is<const char*>() == false)
        return default_value.c_str();
    return paw_print_.getStrValue(idx_);
}

}
