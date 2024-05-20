ULIST= file.proto uattributes.proto umessage.proto upayload.proto uprotocol_options.proto uri.proto ustatus.proto uuid.proto

all : simple libmyprotocol.so mysimple libuprotocol.so

simple : simple.cpp
	g++ --std=c++17 `pkg-config --cflags protobuf` -Iuprotocol_code -o $@ $+ -L. -luprotocol `pkg-config --libs protobuf`


mysimple : mysimple.cpp AnymapWrapper.cpp libmyprotocol.so
	g++ --std=c++17 `pkg-config --cflags protobuf` -Imyprotocol_code -o $@ $+ -L. -lmyprotocol `pkg-config --libs protobuf`

libmyprotocol.so : myprotocol/myprotocol.proto
	protoc --cpp_out=myprotocol_code -Imyprotocol myprotocol.proto
	g++ -shared --std=c++17 -o libmyprotocol.so myprotocol_code/*.cc `pkg-config --libs protobuf`

libuprotocol.so : $(addprefix uprotocol/, $(ULIST))
	protoc --cpp_out=uprotocol_code -Iuprotocol $(ULIST)
	g++ -shared --std=c++17 -o libuprotocol.so -I/usr/local/include uprotocol_code/*.cc -lprotobuf -pthread
