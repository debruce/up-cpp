#pragma once

#include <map>
#include <string>
#include <any>
#include <ostream>
#include <google/protobuf/message.h>

using AnyMap = std::map<std::string, std::any>;

AnyMap protobuf2anymap(const google::protobuf::Message&);
AnyMap protobuf2anymapOptions(const google::protobuf::Message&);
bool anymap2protobuf(const AnyMap& am, google::protobuf::Message&);
void anymapFormat(std::ostream& os, const AnyMap& m, const std::string& pad = "    ", size_t depth=0, bool with_type=false);
