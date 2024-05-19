#include "AnymapWrapper.h"
#include <iostream>
#include "myprotocol_code/myprotocol.pb.h"

using namespace std;
namespace gpb = google::protobuf;

int main(int argc, char *argv[])
{
    if (0) {
        auto ptr = new myprotocol::Outer();
        auto am = protobuf2anymap(*ptr, false);
        anymapFormat(cout, am);
    }

    if (0) {
        auto ptr = new myprotocol::Outer();
        ptr->set_a(100);
        // // ptr->set_b(200);
        // ptr->set_c(300);
        // ptr->set_x(400);
        // ptr->set_d(500);
        cout << ptr->DebugString() << endl;
        // auto am = protobuf2anymapOptions(*ptr);
        auto am = protobuf2anymap(*ptr);
        anymapFormat(cout, am);
    }

    if (1) {
        auto ptr = new myprotocol::Outer();
        AnyMap merge = {
            { "a", 100 },
            { "b", 200 },
            { "c", 300 },
            // { "c", "hello" },
            { "x", AnyMap{
                { "innerAX", "innerAX" },
                { "innerAY", "innerAY" }
            } },
            { "y", AnyMap{
                { "innerBX", "innerBX" },
                { "innerBY", "innerBY" }
            } },
            // { "z", 1234 }
        };

        cout << "before anymap2protobuf" << endl;
        auto passing = anymap2protobuf(merge, *ptr);
        cout << "passing = " << passing << endl;
        cout << ptr->DebugString() << endl;
    }
}