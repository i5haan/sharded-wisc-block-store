#include<iostream>
#include<vector>
#include<thread>
#include "client_lib.h"

using namespace std;

/*
Single Client 
All writes differnt address
*/
int SingleClientConsistencyDiffAddr()
{
    HafsClientFactory client("155.98.36.86:8093", "155.98.36.88:8094");
    //HafsClient client1(grpc::CreateChannel("155.98.36.86:8093", grpc::InsecureChannelCredentials()), "155.98.36.86:8093", false);
    //HafsClient client2(grpc::CreateChannel("155.98.36.88:8094", grpc::InsecureChannelCredentials()), "155.98.36.88:8094", false);
    string res;
    // // client.Read(0, &res);
    // // cout << "Data read: " << res << endl;

    vector<int> UsedAddr;

    for(int i = 0; i < 10000; i++) {
        int CharId = rand()%26;
        string data = string(4096, 'a'+ CharId);
        client.Write(i*4096, data);
        UsedAddr.push_back(i*4096);
        /*client.Read(i*4096, &res);

        if(res!=data)
        {
            cout<<"Read Write Failed"<<endl;
            return 0;
        }*/
    }
    for(int i=0;i<UsedAddr.size();i++)
    {
        if(client.CheckConsistancy(UsedAddr[i])!=true)
        {         
            return 0;
        }
    }

    usleep(10*1000000);
    return 1;
}


/*
Single Client 
All address Same
*/
int SingleClientConsistencySameAddr(int k)
{
    HafsClientFactory client("155.98.36.86:8093", "155.98.36.88:8094");
    //HafsClient client1(grpc::CreateChannel("155.98.36.86:8093", grpc::InsecureChannelCredentials()), "155.98.36.86:8093", false);
    //HafsClient client2(grpc::CreateChannel("155.98.36.88:8094", grpc::InsecureChannelCredentials()), "155.98.36.88:8094", false);
    // // client.Read(0, &res);
    // // cout << "Data read: " << res << endl;
    int Addr = k*4096;
    for(int i = 0; i < 10000; i++) {
        int CharId = rand()%26;
        string data = string(4096, 'a'+ CharId);
        client.Write(Addr, data);
        /*client.Read(i*4096, &res);

        if(res!=data)
        {
            cout<<"Read Write Failed"<<endl;
            return 0;
        }*/
    }

    if(client.CheckConsistancy(Addr)!=true)
        return 0;
    usleep(10*1000000);
    return 1;
}

/*
Multiple Client 
Same Addr
*/

int rtCodeCommon=1;
void ConsistencySameAddr(int k)
{
    int block=4096;
    int numWrites = 10000;
    HafsClientFactory client("155.98.36.86:8093", "155.98.36.88:8094");
    string res;
    for(int i=0;i<numWrites;i++)
    {
        int Addr = 4096;   //(k*block)+block;
        int CharId = rand()%26;
        string data = string(4096, 'a'+ CharId);
        client.Write(Addr, data);
        client.Read(Addr, &res);

        if(res!=data)
        {
            cout<<"Read Write Failed"<<endl;
            rtCodeCommon=0;
            return;
        }
        k++;
    }
}
int MultiClientConsistencySameAddr(int clientCnt)
{
    vector<thread> threads;
    for(int i=0;i<clientCnt;i++)
    {
        threads.push_back(thread(ConsistencySameAddr,0));
    }
    for(int i=0;i<clientCnt;i++)
    {
        threads[i].join();
    }
    if(rtCodeCommon)
        return 1;
    else
        return 0;

}
int main() {
    
    /*int TC = 2;
    int k=0;
    int code = SingleClientConsistencyDiffAddr();
    if(code)
        k++;
    code = SingleClientConsistencySameAddr(24);
    if(code)
        k++;
    if(k!=TC)
        cout<<"Cosistancy failed\n";
    else
        cout<<"Success"<<endl;*/
    for(int i=1;i<=1;i++)
    {
        cout<<"No of Client = "<<i*4<<endl;
        int code = MultiClientConsistencySameAddr(i*4);
        if(code==0)
            cout<<"MultiThread Consistancy failed\n";
        else
            cout<<"MultiThread Consistancy Success"<<endl;
    }
    return 0;
}