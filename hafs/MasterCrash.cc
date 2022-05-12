#include<iostream>
#include<thread>
#include<chrono>
#include "new_client.h"

using namespace std;

#define SIZE 4096

int main() {
    StaticThreeShardFactory client("10.10.1.1:8091", "10.10.1.1:8093", "10.10.1.1:8094", "10.10.1.1:8095", "10.10.1.1:8096", "10.10.1.1:8097", "10.10.1.1:8098");
    int CharId;
    for(int i=1;i<100;i++)
    {
        int addr = i*4096;
        string data = string(4096, 'a');
        client.Write(addr,data);
        string readData;
        client.Read(addr,&readData);
        //cout << readData << endl << endl << data << endl << endl << endl;
        if(data!=readData)
        {
            cout<<"Data Mismatch\n";
        }
        CharId++;
    }
    usleep(1000000);
    client.setShardNo(3);
    client.simulateCrash("Shuffle");
    client.TriggerShuffle();
    
    return 0;
}