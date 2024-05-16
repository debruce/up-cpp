#include <iostream>
#include <vector>
#include <variant>
#include <set>
#include <fstream>
#include <boost/core/demangle.hpp>
#include <nlohmann/json.hpp>
#include "pfiles/myidl.pb.h"

using namespace std;
namespace gpb = google::protobuf;

template <typename T>
string get_type(T& t)
{
    return boost::core::demangle(typeid(t).name());
}

nlohmann::json jsonFromField(const gpb::Reflection* refl, const gpb::FieldDescriptor* field, const gpb::Message& msg)
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
            return nlohmann::json::value_t::null;
    }
}

nlohmann::json protobuf2njson(const gpb::Message& m)
{
    nlohmann::json ret;
    auto desc       = m.GetDescriptor();
    auto refl       = m.GetReflection();
    set<void*> oneof_set;
    for(auto i = 0 ; i < desc->field_count(); i++) {
        const auto field = desc->field(i);
        if (auto cvar = field->containing_oneof()) {
            if (oneof_set.count((void*)cvar) == 0) { // if this not the first field of the oneof, ignore it
                if (cvar->index() < cvar->field_count()) {
                    auto cfield = cvar->field(cvar->index());
                    ret[cvar->name()] = jsonFromField(refl, cfield, m);
                }
                // else {
                //     cout << pad(depth+1) << "oneof " << cvar->name() << " = unset" << endl;
                // }
                oneof_set.insert((void*)cvar); // save oneof handle so repeated fields can be ignored
            }
        }
        else if (field->type() == gpb::FieldDescriptor::TYPE_MESSAGE) {
            auto& submsg = refl->GetMessage(m, field);
            ret[field->name()] = protobuf2njson(submsg);
        }
        else {
            ret[field->name()] = jsonFromField(refl, field, m);
        }
    }
    return ret;
}

void njson2protobuf(const nlohmann::json& j, gpb::Message& m)
{
    auto desc       = m.GetDescriptor();
    auto refl       = m.GetReflection();
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        auto field = desc->FindFieldByName(it.key());
        auto jref = j[it.key()];
        switch (field->cpp_type()) {
            case gpb::FieldDescriptor::CPPTYPE_BOOL: { refl->SetBool(&m, field, jref.get<bool>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_DOUBLE: { refl->SetDouble(&m, field, jref.get<double>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_FLOAT: { refl->SetFloat(&m, field, jref.get<float>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_INT32: { refl->SetInt32(&m, field, jref.get<int32_t>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_UINT32: { refl->SetUInt32(&m, field, jref.get<uint32_t>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_INT64: { refl->SetInt64(&m, field, jref.get<int64_t>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_UINT64: { refl->SetUInt64(&m, field, jref.get<uint64_t>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_STRING: { refl->SetString(&m, field, jref.get<string>()); break; }
            case gpb::FieldDescriptor::CPPTYPE_ENUM: {
                if (jref.type() == nlohmann::json::value_t::string) {
                    auto v = jref.get<string>();
                    auto ptr = field->enum_type()->FindValueByName(v);
                    if (ptr == nullptr) {
                        stringstream ss;
                        ss << "Cannot lookup enum name=" << v << " for field=" << field->name() << endl;
                        throw logic_error(ss.str());
                    }
                    refl->SetEnumValue(&m, field, ptr->number());
                }
                else {
                    refl->SetEnumValue(&m, field, jref.get<int>());
                }
                break;
            }
            case gpb::FieldDescriptor::CPPTYPE_MESSAGE: {
                auto submsg = refl->MutableMessage(&m, field);
                njson2protobuf(jref, *submsg);
                break;               
            }
        }
    }
}

int main(int argc, char *argv[])
{
    ifstream ifs("data.json");
    nlohmann::json jf = nlohmann::json::parse(ifs);
    cout << jf << endl;
    cout << "################################" << endl;
    auto y = new tutorial::Y();
    njson2protobuf(jf, *y);
    cout << "################################" << endl;
    cout << y->DebugString() << endl;
    // i
#if 0 
    auto x = new tutorial::X();
    x->set_dbl(3.1416); // works
    x->set_flt(1.414); // works
    x->set_i32(3); // works
    x->set_i64(4); // works
    x->set_u32(5); // works
    // x->set_u64(6); // works
    x->set_s32(7); // works
    x->set_s64(8); // works
    x->set_f32(9); // works
    x->set_f64(10); // works
    x->set_sf32(11); // works
    x->set_sf64(12); // works
    x->set_bl(true); // works
    x->set_s("hello"); // works
    x->set_bt("bytes"); // works
    x->set_as("as");
    x->set_e(tutorial::InnerEnum::three);
    x->set_ad2(3.1416);
    x->set_ai3(77);
    // x->set_as("hello");
    // prtNode(x);


    auto y = new tutorial::Y();
    y->set_allocated_x(x);
    // prtNode(*y);
    auto j = get_json(*y);
    cout << j.dump(4, ' ', true) << endl;
#endif
}