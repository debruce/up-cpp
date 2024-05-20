#pragma once

#include <map>
#include <string>
#include <any>
#include <memory>
#include <ostream>
#include <google/protobuf/message.h>

using AnyMap = std::map<std::string, std::any>;

AnyMap protobuf2anymap(const google::protobuf::Message&, bool get_options = false);

class Anymap2Protobuf {
public:
    Anymap2Protobuf(const AnyMap& am, google::protobuf::Message&);
    bool is_valid();
private:
    struct Impl;
    std::shared_ptr<Impl>   pImpl;
};

void anymapFormat(std::ostream& os, const AnyMap& m, const std::string& pad = "    ", size_t depth=0, bool with_type=false);
