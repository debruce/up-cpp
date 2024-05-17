#include "AnymapWrapper.h"
#include <iostream>
#include "myprotocol_code/myprotocol.pb.h"

using namespace std;
namespace gpb = google::protobuf;

int main(int argc, char *argv[])
{
    if (0) {
        auto ptr = new myprotocol::Outer();
        auto am = protobuf2anymapOptions(*ptr);
        anymapFormat(cout, am);
    }

    if (1) {
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

    // if constexpr (0) {
    //     auto ptr = new myprotocol::Outer();
    //     AnyMap merge = {
    //         { "a", 100 },
    //         // { "b", 200 },
    //         { "c", 300 },
    //         { "x", 400 },
    //         { "y", 500 },
    //         { "z", 600 }
    //     };

    //     auto passing = anymap2protobuf(merge, *ptr);
    //     cout << "passing = " << passing << endl;
    //     cout << ptr->DebugString() << endl;
    // }
}