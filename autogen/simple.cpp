#include <iostream>
#include <string>
#include <set>
#include <map>
#include <any>
#include <type_traits>
#include <fstream>
#include <boost/core/demangle.hpp>
#include "uprotocol_code/uattributes.pb.h"

using namespace std;
namespace gpb = google::protobuf;

template <typename T>
string get_type(T& t)
{
    return boost::core::demangle(typeid(t).name());
}

string stringFromAny(const any& v)
{
    if (v.type() == typeid(char const*)) return string(any_cast<char const*>(v));
    if (v.type() == typeid(string)) return any_cast<string>(v);
    cerr << "Not string convertable type: " << boost::core::demangle(v.type().name()) << endl;
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

using AnyMap = map<string, any>;
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
    return string("[[ ") + boost::core::demangle(a.type().name()) + " ]]";
}

AnyMap protobuf2anymap(const gpb::Message& m)
{
    AnyMap ret;
    auto desc       = m.GetDescriptor();
    auto refl       = m.GetReflection();
    set<void*> oneof_set;
    for(auto i = 0 ; i < desc->field_count(); i++) {
        const auto field = desc->field(i);
        if (auto oneof_desc = field->containing_oneof()) {
            if (oneof_set.count((void*)oneof_desc) == 0) { // if this not the first field of the oneof, ignore it
                cout << "field_name=" << field->name()
                    << " oneof_desc->field_count()=" << oneof_desc->field_count()
                    << " oneof_desc->index()=" << oneof_desc->index()
                    << " field->index()=" << field->index()
                    << endl;
                for (auto x = 0; x < oneof_desc->field_count(); x++) {
                    auto cfield = oneof_desc->field(x);
                    auto value = anyFromField(refl, cfield, m);
                    cout << "    " << as_string(value) << endl;
                    // cout << "    " << cfield->cpp_type_name()
                    //     << ' ' << cfield->name()
                    //     << ' ' << cfield->number()
                    //     << ' ' << cfield->cpp_type_name()
                    //     << ' ' << cfield->index_in_oneof()
                    //     << endl;
                }
                // if (cvar->index() < cvar->field_count()) {
                //     auto cfield = cvar->field(cvar->index());
                //     cout << "assigned to " << field->name() << endl;
                //     ret[field->name()] = anyFromField(refl, cfield, m);
                // }
                // else {
                //     cout << pad(depth+1) << "oneof " << cvar->name() << " = unset" << endl;
                // }
                oneof_set.insert((void*)oneof_desc); // save oneof handle so repeated fields can be ignored
            }
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_MESSAGE) {
            auto& submsg = refl->GetMessage(m, field);
            ret[field->name()] = protobuf2anymap(submsg);
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_ENUM) {
            const auto& subenum = refl->GetEnum(m, field);
            ret[field->name()] = make_pair(int(subenum->number()), subenum->name());
        }
        else {
            ret[field->name()] = anyFromField(refl, field, m);
        }
    }
    return ret;
}

AnyMap protobuf2anymapOptions(const gpb::Message& m)
{
    AnyMap ret;
    auto desc       = m.GetDescriptor();
    auto refl       = m.GetReflection();
    set<void*> oneof_set;
    for(auto i = 0 ; i < desc->field_count(); i++) {
        const auto field = desc->field(i);
        if (auto cvar = field->containing_oneof()) {
            if (oneof_set.count((void*)cvar) == 0) { // if this not the first field of the oneof, ignore it
                OneofMap om;
                for (auto x = 0; x < cvar->field_count(); x++) {
                    auto cf = cvar->field(x);
                    om.emplace(cf->name(), cf->cpp_type_name());
                }
                ret.emplace(field->name(), om);
                oneof_set.insert((void*)cvar); // save oneof handle so repeated fields can be ignored
            }
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_MESSAGE) {
            auto& submsg = refl->GetMessage(m, field);
            ret.emplace(field->name(), protobuf2anymapOptions(submsg));
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_ENUM) {
            auto enum_desc = field->enum_type();
            EnumMap m;
            for (int x = 0; x < enum_desc->value_count(); x++) {
                auto v = enum_desc->value(x);
                m.emplace(v->number(), v->name());
            }
            ret.emplace(field->name(), m);
        }
        else {
            // ret.emplace(field->name(), anyFromField(refl, field, m));
            ret.emplace(field->name(), field->cpp_type_name());
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
            // cout << "assigning " << amk << " as " << field->cpp_type_name() << endl;
            switch (field->cpp_type()) {
                case gpb::FieldDescriptor::CPPTYPE_BOOL: {
                    refl->SetBool(&msg, field, any_cast<bool>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_DOUBLE: {
                    refl->SetDouble(&msg, field, any_cast<double>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_FLOAT: {
                    refl->SetFloat(&msg, field, any_cast<float>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_INT32: {
                    refl->SetInt32(&msg, field, any_cast<int32_t>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_UINT32: {
                    if (amv.type() == typeid(int)) refl->SetUInt32(&msg, field, any_cast<int>(amv));
                    else refl->SetUInt32(&msg, field, any_cast<uint32_t>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_INT64: {
                    refl->SetInt64(&msg, field, any_cast<int64_t>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_UINT64: {
                    if (amv.type() == typeid(int)) refl->SetUInt64(&msg, field, any_cast<int>(amv));
                    else refl->SetUInt64(&msg, field, any_cast<uint64_t>(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_STRING: {
                    refl->SetString(&msg, field, stringFromAny(amv));
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_ENUM: {
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
                    break;
                }
                case gpb::FieldDescriptor::CPPTYPE_MESSAGE: {
                    auto submsg = refl->MutableMessage(&msg, field);
                    anymap2protobuf(any_cast<AnyMap>(amv), *submsg);
                    break;               
                }
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

void anymapFormat(ostream& os, const AnyMap& m, const string& pad = "    ", size_t depth=0, bool with_type=false)
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
                os << ", " << boost::core::demangle(v.type().name());
            }
            os << '\n';
        }
    }
    if (depth == 0) os << padX(pad, depth) << "}\n";
}

int main(int argc, char *argv[])
{
    auto ptr = new uprotocol::v1::UAttributes();
    // auto am_options = protobuf2anymapOptions(*ptr);
    // anymapFormat(cout, am_options);
    // cout << "#########################" << endl;

    // AnyMap merge;
    // merge["ttl"] = 5;
    AnyMap merge = {
        { "ttl", 5 },
        { "token", "hello world" },
        { "id", AnyMap{
            {"lsb", 6},
            {"msb", 7}
        }},
        {"priority", "UPRIORITY_CS4" }
    };
    // anymapFormat(cout, merge);
    // cout << endl;

    auto passing = anymap2protobuf(merge, *ptr);
    cout << "passing = " << passing << endl;
    cout << "##########################" << endl;
    cout << ptr->DebugString() << endl;
    cout << "##########################" << endl;
    auto am2 = protobuf2anymap(*ptr);
    anymapFormat(cout, am2);
}