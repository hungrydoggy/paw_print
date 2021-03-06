#include "./paw_print.h"

#include <iomanip>
#include <sstream>


namespace paw_print {


using std::stringstream;
using std::to_string;

PawPrint::Cursor::Cursor ()
:paw_print_(null),
 idx_(-1) {
}

PawPrint::Cursor::Cursor (
        const PawPrint *paw_print, int idx, const shared_ptr<PawPrint> &holdable)
:paw_print_(paw_print),
 idx_(idx),
 holder_(holdable){
}

PawPrint::Cursor::Cursor (const Cursor &cursor)
:paw_print_(cursor.paw_print_),
 idx_(cursor.idx_),
 holder_(cursor.holder_){
}

const PawPrint::Cursor& PawPrint::Cursor::operator = (const Cursor &cursor) {
    paw_print_ = cursor.paw_print_;
    idx_       = cursor.idx_;
    holder_    = cursor.holder_;
    return *this;
}

DataType PawPrint::Cursor::type () const {
    if (idx_ < 0)
        return Data::TYPE_NONE;

	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).type();

    return paw_print_->type(idx_);
}

template<> bool PawPrint::Cursor::is<bool       > () const { return type() == Data::TYPE_BOOL;   }
template<> bool PawPrint::Cursor::is<int        > () const { return type() == Data::TYPE_INT;    }
template<> bool PawPrint::Cursor::is<float      > () const { return type() == Data::TYPE_DOUBLE; }
template<> bool PawPrint::Cursor::is<double     > () const { return type() == Data::TYPE_DOUBLE; }
template<> bool PawPrint::Cursor::is<const char*> () const { return type() == Data::TYPE_STRING; }
template<> bool PawPrint::Cursor::is<string     > () const { return type() == Data::TYPE_STRING; }

template<> bool PawPrint::Cursor::isConvertable<bool       > () const {
    return is<bool>() || isConvertable<double>() || isConvertable<string>();
}
template<> bool PawPrint::Cursor::isConvertable<int        > () const {
    return is<int>();
}
template<> bool PawPrint::Cursor::isConvertable<float      > () const {
    return is<double>() || is<int>();
}
template<> bool PawPrint::Cursor::isConvertable<double     > () const {
    return is<double>() || is<int>();
}
template<> bool PawPrint::Cursor::isConvertable<const char*> () const {
    return is<const char*>() || is<double>() || is<int>();
}
template<> bool PawPrint::Cursor::isConvertable<string     > () const {
    return is<string>() || is<double>() || is<int>() || is<bool>();
}

bool PawPrint::Cursor::isNull () const {
    return type() == Data::TYPE_NULL;
}

bool PawPrint::Cursor::isSequence () const {
    return type() == Data::TYPE_SEQUENCE;
}

bool PawPrint::Cursor::isMap () const {
    return type() == Data::TYPE_MAP;
}
bool PawPrint::Cursor::isKeyValuePair () const {
    return type() == Data::TYPE_KEY_VALUE_PAIR;
}

template<>
bool PawPrint::Cursor::get<bool> (bool default_value) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).get(default_value);

	if (is<bool>() == true)
		return paw_print_->getData<char>(idx_ + sizeof(DataType)) != 0;
	else if (isConvertable<double>() == true)
		return get(0.0) != 0.0;
	else if (isConvertable<string>() == true)
		return get("") == "true";
	else
		return default_value;
}

template<>
float PawPrint::Cursor::get<float>(float default_value) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).get(default_value);

	if      (is<double>() == true)
		return (float)paw_print_->getData<double>(idx_ + sizeof(DataType));
	else if (is<int   >() == true)
		return (float)paw_print_->getData<int   >(idx_ + sizeof(DataType));
	else
		return default_value;
}

template<>
double PawPrint::Cursor::get<double> (double default_value) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).get(default_value);

    if      (is<double>() == true)
        return paw_print_->getData<double>(idx_ + sizeof(DataType));
    else if (is<int   >() == true)
        return paw_print_->getData<int   >(idx_ + sizeof(DataType));
    else
        return default_value;
}

string PawPrint::Cursor::get (const char *default_value) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).get(default_value);

	if      (is<bool>() == true)
		return to_string(paw_print_->getData<bool  >(idx_ + sizeof(DataType)));
	else if (is<double>() == true)
        return to_string(paw_print_->getData<double>(idx_ + sizeof(DataType)));
    else if (is<int   >() == true)
        return to_string(paw_print_->getData<int   >(idx_ + sizeof(DataType)));
    else if (is<string>() == true)
        return paw_print_->getStrValue(idx_);
    else
        return string(default_value);
}

string PawPrint::Cursor::get (const string &default_value) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).get(default_value);

    if      (is<bool>() == true)
		return to_string(paw_print_->getData<bool  >(idx_ + sizeof(DataType)));
    else if (is<double>() == true)
        return to_string(paw_print_->getData<double>(idx_ + sizeof(DataType)));
    else if (is<int   >() == true)
        return to_string(paw_print_->getData<int   >(idx_ + sizeof(DataType)));
    else if (is<string>() == true)
        return paw_print_->getStrValue(idx_);
    else
        return default_value;
}

PawPrint::Cursor PawPrint::Cursor::operator[] (int idx) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).operator[](idx);

	if (isSequence() == false)
		return Cursor(paw_print_, -1, holder_);

    auto &data_idxs = paw_print_->getDataIdxsOfSequence(idx_);
    if (idx < 0 || idx >= data_idxs.size())
        return Cursor(paw_print_, -1, holder_);

    return Cursor(paw_print_, data_idxs[idx], holder_);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const char *key) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).operator[](key);

    if (isMap() == false)
        return Cursor(paw_print_, -1, holder_);

    auto &sorted_data_idxs = paw_print_->getSortedDataIdxsOfMap(idx_);
    int pair_idx = paw_print_->findRawIdxOfValue(sorted_data_idxs, 0, sorted_data_idxs.size() - 1, key);
    if (pair_idx < 0)
        return Cursor(paw_print_, -1, holder_);

    auto value_idx = paw_print_->getValueRawIdxOfPair(pair_idx);
    return Cursor(paw_print_, value_idx, holder_);
}

PawPrint::Cursor PawPrint::Cursor::operator[] (const string &key) const {
    return operator [] (key.c_str());
}

PawPrint::Cursor PawPrint::Cursor::getKeyValuePair (int idx) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getKeyValuePair(idx);

    if (isMap() == false)
		return Cursor(paw_print_, -1, holder_);

    auto &data_idxs = paw_print_->getDataIdxsOfMap(idx_);
	return PawPrint::Cursor(paw_print_, data_idxs[idx], holder_);
}

int PawPrint::Cursor::size () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).size();

    if (isSequence() == true)
        return paw_print_->getDataIdxsOfSequence(idx_).size();
    else if (isMap() == true)
        return paw_print_->getDataIdxsOfMap(idx_).size();
    else
        return 1;
}

string PawPrint::Cursor::toString (int indent, int indent_inc, bool ignore_indent) const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).toString(indent, indent_inc, ignore_indent);

    stringstream ss;

    if (ignore_indent == false) {
        for (int i=0; i<indent; ++i)
            ss << " ";
    }

    switch (type()) {
        case PawPrint::Data::TYPE_NONE:
            ss << "NONE" << endl;
            break;
        case PawPrint::Data::TYPE_NULL:
            ss << "null" << endl;
            break;
        case PawPrint::Data::TYPE_BOOL:
            ss << ((get(false) == true)? "true": "false") << endl;
            break;
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
            if (size() <= 0)
                ss << "[ ]" << endl;
			for (int i = 0; i < size(); ++i) {
				
				if (i != 0) {
					for (int i = 0; i<indent; ++i)
						ss << " ";
				}

				auto child = (*this)[i];
				auto need_new_line = child.isSequence() || (child.isMap() && child.size() > 1);
				ss << "- ";
				if (need_new_line == true) {
					ss << "\n";
					for (int i = 0; i<indent + indent_inc; ++i)
						ss << " ";
				}
				ss << child.toString(indent + indent_inc, indent_inc, true);
			}
            break;
        case PawPrint::Data::TYPE_MAP:
            if (size() <= 0)
                ss << "{ }" << endl;
            for (int i=0; i<size(); ++i) {
				if (i != 0) {
					for (int i = 0; i<indent; ++i)
						ss << " ";
				}
                ss << getKeyOfPair(i) << " :" << endl;
                ss << getValueOfPair(i).toString(indent + indent_inc, indent_inc);
            }
            break;
        default:
            // TODO err
            cout << "err: cannot cursor convert to string type \'"
                    << to_string(type()) << "\'" << endl;
            return "";
    }

    return ss.str();
}

const string & PawPrint::Cursor::getName () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getName();

	return paw_print_->name();
}

int PawPrint::Cursor::getColumn () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getColumn();

    return paw_print_->getColumn(idx_);
}

int PawPrint::Cursor::getLine () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getLine();

    return paw_print_->getLine(idx_);
}

const char* PawPrint::Cursor::getKey () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getKey();

    if (isKeyValuePair() == true) {
        auto key_idx = paw_print_->getKeyRawIdxOfPair(idx_);
        return paw_print_->getStrValue(key_idx);
    }else if (isMap() == true && size() > 0) {
		return getKeyOfPair(0);
    }

    return null;
}

PawPrint::Cursor PawPrint::Cursor::getValue () const {
	if (paw_print_->isReference(idx_) == true)
		return paw_print_->_getReference(idx_).getValue();

    if (isKeyValuePair() == true) {
        auto value_idx = paw_print_->getValueRawIdxOfPair(idx_);
        return Cursor(paw_print_, value_idx, holder_);
    }else if (isMap() == true && size() > 0) {
        return getValueOfPair(0);
    }

    return Cursor(paw_print_, -1, holder_);
}

}
