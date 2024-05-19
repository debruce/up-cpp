#include "AnymapWrapper.h"
#include <boost/core/demangle.hpp>

using namespace std;
namespace gpb = google::protobuf;

template <typename T>
string get_type(const T& t)
{
    return boost::core::demangle(typeid(t).name());
}

string stringFromAny(const any& v)
{
    if (v.type() == typeid(char const*)) return string(any_cast<char const*>(v));
    if (v.type() == typeid(string)) return any_cast<string>(v);
    cerr << "Not string convertable type: " << v.type().name() << endl;
    return "Bad cast";
}

any anyFromField(const gpb::Reflection* refl, const gpb::FieldDescriptor* field, const gpb::Message& msg)
{
    using namespace google::protobuf;
    switch (field->type()) {
        case FieldDescriptor::TYPE_DOUBLE: return  refl->GetDouble(msg, field);
        case FieldDescriptor::TYPE_FLOAT: return  refl->GetFloat(msg, field);
        case FieldDescriptor::TYPE_INT64: return  refl->GetInt64(msg, field);
        case FieldDescriptor::TYPE_SINT64: return  refl->GetInt64(msg, field);
        case FieldDescriptor::TYPE_INT32: return  refl->GetInt32(msg, field);
        case FieldDescriptor::TYPE_SINT32: return  refl->GetInt32(msg, field);
        case FieldDescriptor::TYPE_UINT64: return  refl->GetUInt64(msg, field);
        case FieldDescriptor::TYPE_UINT32: return  refl->GetUInt32(msg, field);
        case FieldDescriptor::TYPE_BOOL: return  refl->GetBool(msg, field);
        case FieldDescriptor::TYPE_FIXED64: return  refl->GetUInt64(msg, field);
        case FieldDescriptor::TYPE_FIXED32: return  refl->GetUInt32(msg, field);
        case FieldDescriptor::TYPE_SFIXED64: return  refl->GetInt64(msg, field);
        case FieldDescriptor::TYPE_SFIXED32: return  refl->GetInt32(msg, field);
        case FieldDescriptor::TYPE_STRING: return  refl->GetString(msg, field);
        case FieldDescriptor::TYPE_BYTES: return  refl->GetString(msg, field);
        case FieldDescriptor::TYPE_ENUM: {
            auto e = refl->GetEnum(msg, field);
            return std::make_pair(field->type(), e->name());
        }
        default:
            return nullptr;
    }
}

void anyToField(
    gpb::FieldDescriptor::CppType cpp_type,
    const gpb::Reflection* refl,
    const gpb::FieldDescriptor* field,
    const any& a,
    gpb::Message& msg)
{
    using FD = gpb::FieldDescriptor;
    switch (cpp_type) {
        case FD::CPPTYPE_BOOL: {
            refl->SetBool(&msg, field, any_cast<bool>(a));
            return;
        }
        case FD::CPPTYPE_DOUBLE: {
            refl->SetDouble(&msg, field, any_cast<double>(a));
            return;
        }
        case FD::CPPTYPE_FLOAT: {
            refl->SetFloat(&msg, field, any_cast<double>(a));
            return;
        }
        case FD::CPPTYPE_INT32: {
            refl->SetInt32(&msg, field, any_cast<int>(a));
            return;
        }
        case FD::CPPTYPE_INT64: {
            refl->SetInt64(&msg, field, any_cast<int>(a));
            return;
        }
        case FD::CPPTYPE_STRING: {
            refl->SetString(&msg, field, stringFromAny(a));
            return;
        }
        case FD::CPPTYPE_UINT32: {
            refl->SetUInt32(&msg, field, any_cast<int>(a));
            return;
        }
        case FD::CPPTYPE_UINT64: {
            refl->SetUInt64(&msg, field, any_cast<int>(a));
            return;
        }
        case FD::CPPTYPE_ENUM:
        case FD::CPPTYPE_MESSAGE:
            return;
    }
}

using EnumMap = map<int, string>;
using OneofMap = map<string, string>;

string as_string(const any& a)
{
    if (a.type() == typeid(double)) return to_string(any_cast<double>(a));
    if (a.type() == typeid(float)) return to_string(any_cast<float>(a));
    if (a.type() == typeid(bool)) return to_string(any_cast<bool>(a));
    if (a.type() == typeid(int8_t)) return to_string(any_cast<int8_t>(a));
    if (a.type() == typeid(int16_t)) return to_string(any_cast<int16_t>(a));
    if (a.type() == typeid(int32_t)) return to_string(any_cast<int32_t>(a));
    if (a.type() == typeid(int64_t)) return to_string(any_cast<int64_t>(a));
    if (a.type() == typeid(uint8_t)) return to_string(any_cast<uint8_t>(a));
    if (a.type() == typeid(uint16_t)) return to_string(any_cast<uint16_t>(a));
    if (a.type() == typeid(uint32_t)) return to_string(any_cast<uint32_t>(a));
    if (a.type() == typeid(uint64_t)) return to_string(any_cast<uint64_t>(a));
    if (a.type() == typeid(const char*)) return string("\"") + string(any_cast<const char*>(a)) + '"';
    if (a.type() == typeid(string)) return string("\"") + any_cast<string>(a) + '"';
    if (a.type() == typeid(pair<int,string>)) {
        auto p = any_cast<pair<int,string>>(a);
        stringstream ss;
        ss << "Enum(" << p.first << '=' << p.second << ')';
        return ss.str();
    }
    if (a.type() == typeid(EnumMap)) {
        auto em = any_cast<EnumMap>(a);
        string s;
        if (em.size() > 0) {
            stringstream ss;
            ss << "EnumMap(";
            for (const auto& [k,v] : em) {
                ss << k << '=' << v << ", ";
            }
            s = ss.str();
            s = s.substr(0, s.size()-2);
            s += ')';
        }
        else {
            s = "EnumMap()";
        }
        return s;
    }
    if (a.type() == typeid(OneofMap)) {
        auto om = any_cast<OneofMap>(a);
        string s;
        if (om.size() > 0) {
            stringstream ss;
            ss << "Oneof(";
            for (const auto& [k, v] : om) {
                ss << k << '=' << v << ", ";
            }
            s = ss.str();
            s = s.substr(0, s.size()-2);
            s += ')';
        }
        else {
            s = "Oneof()";
        }
        return s;
    }
    return string("[[ ") + a.type().name() + " ]]";
}

AnyMap protobuf2anymap(const gpb::Message& m, bool get_options)
{
    AnyMap ret;
    auto desc       = m.GetDescriptor();
    auto refl       = m.GetReflection();
    set<void*> oneof_set;
    for(auto i = 0 ; i < desc->field_count(); i++) {
        const auto field = desc->field(i);
        if (auto oneof_desc = field->containing_oneof()) {
            if (oneof_set.count((void*)oneof_desc) == 0) {
                if (get_options) {
                    OneofMap om;
                    for (auto x = 0; x < oneof_desc->field_count(); x++) {
                        auto cf = oneof_desc->field(x);
                        if (cf->type() == gpb::FieldDescriptor::TYPE_MESSAGE) {
                            om.emplace(cf->name(), cf->message_type()->name());
                        }
                        else if (cf->type() == gpb::FieldDescriptor::TYPE_ENUM) {
                            om.emplace(cf->name(), cf->enum_type()->name());
                        }
                        else {
                            om.emplace(cf->name(), cf->cpp_type_name());
                        }
                    }
                    ret.emplace(oneof_desc->name(), om);
                    oneof_set.insert((void*)oneof_desc); // save oneof handle so repeated fields can be ignored
                }
                else {
                    for (auto x = 0; x < oneof_desc->field_count(); x++) {
                        auto cf = oneof_desc->field(x);
                        if (refl->HasField(m, cf)) {
                            auto value = anyFromField(refl, cf, m);
                            ret.emplace(cf->name(), value);
                        }
                    }
                    oneof_set.insert((void*)oneof_desc); // save oneof handle so repeated fields can be ignored
                }
            }
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_MESSAGE) { 
            auto& submsg = refl->GetMessage(m, field); // for nested messages, option case and value case are to just recurse
            ret.emplace(field->name(), protobuf2anymap(submsg, get_options));
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_ENUM) {
            if (get_options) { // options is a map of enum key,int pairs
                auto enum_desc = field->enum_type();
                EnumMap m;
                for (int x = 0; x < enum_desc->value_count(); x++) {
                    auto v = enum_desc->value(x);
                    m.emplace(v->number(), v->name());
                }
                ret.emplace(field->name(), m);
            }
            else { // non-option case returns a pair including the int and string names
                const auto& enum_value = refl->GetEnum(m, field);
                ret.emplace(field->name(), make_pair(int(enum_value->number()), enum_value->name()));
            }
        }
        else {
            if (get_options) { // if its a scalar field, option is just the C++ type name
                ret.emplace(field->name(), field->cpp_type_name());
            }
            else {  // get the C++ value
                ret.emplace(field->name(), anyFromField(refl, field, m));
            }
        }
    }
    return ret;
}

bool anymap2protobuf(const AnyMap& am, gpb::Message& msg)
{
    try {
        auto desc       = msg.GetDescriptor();
        auto refl       = msg.GetReflection();
        for (const auto& [amk, amv] : am) {
            auto field = desc->FindFieldByName(amk);
            if (field == nullptr) return false;

            if (field->cpp_type() == gpb::FieldDescriptor::CPPTYPE_ENUM) {
                if (amv.type() == typeid(string) || amv.type() == typeid(char const*)) {
                    auto v = stringFromAny(amv);
                    auto ptr = field->enum_type()->FindValueByName(v);
                    if (ptr == nullptr) {
                        stringstream ss;
                        ss << "Cannot lookup enum name=" << v << " for field=" << field->name() << endl;
                        throw logic_error(ss.str());
                    }
                    refl->SetEnumValue(&msg, field, ptr->number());
                }
                else {
                    refl->SetEnumValue(&msg, field, any_cast<int>(amv));
                }
            }
            else if (field->cpp_type() == gpb::FieldDescriptor::CPPTYPE_MESSAGE) {
                auto submsg = refl->MutableMessage(&msg, field);
                anymap2protobuf(any_cast<AnyMap>(amv), *submsg);
            }
            else {
                anyToField(field->cpp_type(), refl, field, amv, msg);
            }
        }
    }
    catch (const std::exception& ex) {
        cerr << "Caught " << ex.what() << endl;
        return false;
    }
    return true;
}

string padX(const string& pad, size_t x)
{
    string ret;
    while (x > 0) {
        ret += pad;
        x--;
    }
    return ret;
}

void anymapFormat(ostream& os, const AnyMap& m, const string& pad, size_t depth, bool with_type)
{
    if (depth == 0) os << padX(pad, depth) << "{\n";
    for (const auto& [k, v] : m) {
        if (v.type() == typeid(AnyMap)) {
            os << padX(pad, depth+1) << k << " = " << "{\n";
            anymapFormat(os, any_cast<AnyMap>(v), pad, depth+2);
            os << padX(pad, depth+1) << "}\n";
        }
        else {
            os << padX(pad, depth+1) << k << " = " << as_string(v);
            if (with_type) {
                os << ", " << v.type().name();
            }
            os << '\n';
        }
    }
    if (depth == 0) os << padX(pad, depth) << "}\n";
}
