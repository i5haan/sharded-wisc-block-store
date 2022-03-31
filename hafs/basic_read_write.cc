#include<iostream>
#include "client_lib.h"

using namespace std;


int main() {
    HafsClientFactory client("0.0.0.0:8090", "0.0.0.0:8091");
    // HafsClient client(grpc::CreateChannel("0.0.0.0:8090", grpc::InsecureChannelCredentials()), "0.0.0.0:8090", false);
    // HafsClient client2(grpc::CreateChannel("0.0.0.0:8091", grpc::InsecureChannelCredentials()), "0.0.0.0:8091", false);
    // string res;
    // // client.Read(0, &res);
    // // cout << "Data read: " << res << endl;

    client.Write(0, string(4096, 'a'));
    client.Write(4096, string(4096, 'b'));
    client.Write(8192, string(4096, 'c'));
    client.Write(8192 + 4096, string(4096, 'd'));
    client.Write(8192 + 8192, string(4096, 'e'));
    client.Write(8192 + 8192 + 4096, string(4096, 'f'));

    // client2.Read(0, &res);
    // cout << "Data read: " << res << endl;
    // client2.Read(4096, &res);
    // cout << "Data read: " << res << endl;
    // client2.Read(8192, &res);
    // cout << "Data read: " << res << endl;
    // client2.Read(8192 + 4096, &res);
    // cout << "Data read: " << res << endl;
    // client2.Read(8192 + 8192, &res);
    // cout << "Data read: " << res << endl;
    // client2.Read(8192 + 8192 + 4096, &res);
    // cout << "Data read: " << res << endl;



    // // client.Write(4096, string(4096, 'b'));
    // // client.Write(8192, string(4096, 'c'));

    // // string res2;
    // // client.Read(8392704, &res2);
    // // cout << "Data read: " << res2 << endl;

    usleep(10*1000000);
    return 0;
}