#include "client_lib.h"
#include "hafs.grpc.pb.h"
#include<bits/stdc++.h>
#define MAXPOD 10
using namespace std;

using ::Connection;
using ::Response;
using ::Request;
using ::MStatus;
using ::MasterState;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::Status;

#define BLK_SIZE 4096

class StaticThreeShardFactory {
    int numShards;
    bool up;
    HafsClientFactory one;
    HafsClientFactory two;
    HafsClientFactory three;
    std::unique_ptr<Master::Stub> master_stub_;

    public:
        StaticThreeShardFactory(
            string master,
            string oneP, string oneB,
            string twoP, string twoB,
            string threeP, string threeB) : one(oneP, oneB), two(twoP, twoB), three(threeP, threeB), master_stub_(Master::NewStub(grpc::CreateChannel(master, grpc::InsecureChannelCredentials()))) {
                getAndSetState();
        }

        int getShards() {
            return numShards;
        }

        void getAndSetState() {
            Request req;
            MasterState res;

            ClientContext context;

            Status status = master_stub_->GetState(&context, req, &res);

            if (status.ok()) {
                cout << "[StaticThreeShardFactory] Master State" << endl;
                if(res.shards() == MasterState_Shards_TWO) {
                    numShards = 2;
                } else {
                    numShards = 3;
                }
                if(res.status() == MasterState_Status_VALID) {
                    up = true;
                } else {
                    up = false;
                }
            }
        }

        void setState() {

        }

        int getLogicalAdd(int addr,int n)
        {
            int blockID = addr/BLK_SIZE;
            int logicalBlockID =  floor(blockID/n);
            return (logicalBlockID*BLK_SIZE);
        }
        bool Write(int addr, std::string data) 
        {
            if(addr%BLK_SIZE!=0)
            {
                cout<<"UnallignedAddr"<<endl;
            }
            int shardID = (addr/BLK_SIZE)%numShards;
            int logicalAddr = getLogicalAdd(addr,numShards);
            if(shardID==0)
            {
                one.Write(logicalAddr,addr,data);
            }
            else if(shardID==1)
            {
                two.Write(logicalAddr,addr,data);
            }
            else
            {
                three.Write(logicalAddr,addr,data);
            }
        }
        bool Read(int addr, std::string *data) 
        {
            if(addr%BLK_SIZE!=0)
            {
                cout<<"UnallignedAddr"<<endl;
            }
            int shardID = (addr/BLK_SIZE)%numShards;
            int logicalAddr = getLogicalAdd(addr,numShards);
            if(shardID==0)
            {
                one.Read(logicalAddr,data);
            }
            else if(shardID==1)
            {
                two.Read(logicalAddr,data);
            }
            else
            {
                three.Read(logicalAddr,data);
            }
        }


};