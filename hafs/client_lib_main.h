#include "client_lib.h"
#include<bits/stdc++.h>
#define MAXPOD 10
using namespace std;

using ::Connection;
using ::Response;
using ::Request;
using ::MStatus;

#define BLOCK_SIZE 4096
unordered_map<int, HafsClientFactory> shard_servers;

class HafsClientShardFactory {

    public:
        HafsClientShardFactory(string master_address){
            master_stub_ = Master::NewStub(grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials()));
            cout << "[HafsCLient] Starting Hafs Client Instance!" <<std::endl;
            this->master_address = master_address;

            //check Master status?
            this->state = true;
            // Get all shards from Master for the first time
            vector<Connection> shards = getShards();

            // Connect to all shard servers for the first time
            connectToShards(shards);
            std::thread thread_object(&HafsClientShardFactory::checkHealth, this);
            thread_object.detach();

        }

        void connectToShards(vector<Connection> &shards){
            if (shards.size() == 0) {
                cout << "[HafsCLient] [Error] No shards found in Master" << endl;
            }
            else{
                cout << "[HafsCLient] Found " << shards.size() << " active shards in Master" << endl;
                //Connect client to all shards
                for(int i = 0; i < shards.size(); i++) {
                    if (shard_servers.find(shards[i].id()) == shard_servers.end()) {
                        shard_servers.insert({shards[i].id(),HafsClientFactory(shards[i].primaryaddr(),  shards[i].backupaddr())});
                        // shard_servers.insert({shards[i].id(),HafsClientFactory(grpc::CreateChannel(shards[i].primaryaddr(), grpc::InsecureChannelCredentials()), shards[i].primaryaddr(), grpc::CreateChannel(shards[i].backupaddr(), grpc::InsecureChannelCredentials()),  shards[i].backupaddr())});
                        //shard_servers[shards[i].id()] = HafsClientFactory(shards[i].primaryaddr(), shards[i].backupaddr());
                    }
                }
            }



        }
        bool SetStatus(bool flag)
        {
            MStatus request;
            Response reply;
            if(flag)
            {
                request.set_state(MStatus_Status_VALID);
            }
            else
            {
                request.set_state(MStatus_Status_INVALID);
            }

            ClientContext context;

            Status status = master_stub_->SetState(&context, request, &reply);

            if (!status.ok()) {
                std::cout << "[HafsMasterCLient] AckMaster: error code[" << status.error_code() << "]: " << status.error_message() << std::endl;
                return false;
            }
            return true;            
        }
        void GetStatus()
        {
            Request request;
            Response reply;

            ClientContext context;

            Status status = master_stub_->GetState(&context, request, &reply);

            if (!status.ok()) {
                std::cout << "[HafsMasterCLient] GetStatus: error code[" << status.error_code() << "]: " << status.error_message() << std::endl;
            }
            if(reply.status()==Response_Status_VALID)
            {
                setState(true);
            }
            else
            {
                setState(false);
            }           
        }

        void checkHealth()
        {
            while(true)
            {
                usleep(100000);
                GetStatus();
                std::cout<<"[HafsMasterCLient] GetStatus state :"<<getState()<<std::endl;
            }
        }
 /*       void balanceLoad()
        {
            while(true)
            {
                usleep(10000000);
                int oldShardCount = this->shard_servers.size();
                vector<Connection> shards = getShards();
                int newShardCount = shards.size();
                if(newShardCount>oldShardCount)
                {
                    //Checkfor BlockCount in Shards
                    bool ShuffleFlag=false;
                    for(int i=0;i<oldShardCount;i++)
                    {
                        if(this->shard_servers[i].CanShuffle())
                        {
                            ShuffleFlag=true;
                            break;
                        }
                    }   
                    if(ShuffleFlag)
                    {
                        for(int i=0;i<oldShardCount;i++)
                        {
                            //This is sequential thing about making this request parallel
                            ShuffleFlag = ShuffleFlag & this->shard_servers[i].TriggerShuffle(newShardCount);
                        } 
                    }
                    if(ShuffleFlag)
                    {
                        //Update local shard client list with new connection
                        this->shard_servers.push_back(HafsClientFactory(shards[newShardCount-1].primaryaddr(), shards[newShardCount-1].backupaddr()));
                    }
                }


                
            }
        }
*/
        vector<Connection> getShards() {
            vector<Connection> shards;
            // Get all shards from Master
            Request request;
            Connection pods;
            ClientContext context;
            std::unique_ptr<ClientReader<Connection> > reader(
                master_stub_->ActiveConnections(&context, request));
            while (reader->Read(&pods)) {
                shards.push_back(pods);
            }
            Status status = reader->Finish();

            cout << "Error message: [" << status.error_message() << "], error_code: [" << status.error_code() << "]" << endl;

            if(status.ok()) {
                cout << "[HafsCLient] Recieved Shards from Master" << std::endl;
            } else {
                cout << "[HafsCLient] Failed to Recieve Shards from Master" << std::endl;
            }

            return shards;
        }

        int AddShard(std::string pAddress,std::string bAddress)
        {
            Connection request;
            ConResponse reply;
            request.set_primaryaddr(pAddress);
            request.set_backupaddr(bAddress);

            ClientContext context;

            Status status = master_stub_->AckMaster(&context, request, &reply);

            if (!status.ok()) {
                std::cout << "[HafsMasterCLient] AckMaster: error code[" << status.error_code() << "]: " << status.error_message() << std::endl;
                return -1;
            }
            return reply.shardcnt();
        }

        bool Write(int addr, std::string data) {

            int block1 = addr / BLOCK_SIZE;
            int offset = addr % BLOCK_SIZE;
            int block2 = (offset == 0) ? -1 : block1 + 1;

            pair<int,int> shard_info = getShardNmbrAndLogicalAddr(block1);
            int shard_id = shard_info.first, shard_id2;
            int shard_block1 = shard_info.second, shard_block2;
            string data2;

            if(block2 != -1) { //unaligned write
                
                pair<int,int> shard_info2 = getShardNmbrAndLogicalAddr(block2);
                shard_id2 = shard_info2.first;
                shard_block2 = shard_info2.second;

                data2 = data.substr(BLOCK_SIZE - (offset), data.size() - (BLOCK_SIZE - (offset)));
                data = data.substr(0, BLOCK_SIZE - (offset));
            }

            return shard_servers.at(shard_id).Write(shard_block1 * BLOCK_SIZE + offset, addr, data) && block2 != -1 ? shard_servers.at(shard_id2).Write(shard_block2 * BLOCK_SIZE, block2 * BLOCK_SIZE, data2) : true;
        }

        bool Read(int addr, std::string& data) {
            int block1 = addr / BLOCK_SIZE;
            int offset = addr % BLOCK_SIZE;
            int block2 = (offset == 0) ? -1 : block1 + 1;

            pair<int,int> shard_info = getShardNmbrAndLogicalAddr(block1);
            int shard_id = shard_info.first, shard_id2;
            int shard_block1 = shard_info.second, shard_block2;

            string data2;
            string data1;
            if(block2 != -1) { //unaligned read
                pair<int,int> shard_info2 = getShardNmbrAndLogicalAddr(block2);
                shard_id2 = shard_info2.first;
                shard_block2 = shard_info2.second;

            }
            if (shard_servers.at(shard_id).Read(shard_block1 * BLOCK_SIZE + offset, &data1) && block2 != -1 ? shard_servers.at(shard_id2).Read(shard_block2 * BLOCK_SIZE, &data2) : true) {
                if (block2 != -1) {
                    data = data1 + data2.substr(0, (offset));  /* check: server should send BLOCK_SIZE data for second request because it treats it as aligned read?
                                                                             shouldn't be an issue, because only 4k data, otherwise, add another arg for rpc - size? */
                                                                         
                }
                else {
                    data = data1;
                }
                return true;
            }
            return false;
        }

        pair<int, int>  getShardNmbrAndLogicalAddr(int block) {
            cout << "[HafsCLient] Get shard number for block[" << block << "]" << endl;
            return {block % shard_servers.size() , float(block) / shard_servers.size()};
        }
        void setState(bool flag)
        {
            state = flag;
        }
        bool getState()
        {
            return state;
        }

    private:
        std::unique_ptr<Master::Stub> master_stub_;
        string master_address;
        bool state;

        
};