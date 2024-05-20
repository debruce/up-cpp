all : simple libmyprotocol.so mysimple

simple : simple.cpp
	g++ --std=c++17 `pkg-config --cflags protobuf` -Iuprotocol_code -o $@ $+ -L. -luprotocol `pkg-config --libs protobuf`


mysimple : mysimple.cpp AnymapWrapper.cpp libmyprotocol.so
	g++ --std=c++17 `pkg-config --cflags protobuf` -Imyprotocol_code -o $@ $+ -L. -lmyprotocol `pkg-config --libs protobuf`

libmyprotocol.so : myprotocol/myprotocol.proto
	protoc --cpp_out=myprotocol_code -Imyprotocol myprotocol.proto
	g++ -shared --std=c++17 -o libmyprotocol.so myprotocol_code/*.cc `pkg-config --libs protobuf`