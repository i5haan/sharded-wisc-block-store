#include "client_lib.h"
#define MAXPOD 10
using namespace std;

using ::Connection;
using ::Response;

class HafsClientShardFactory {

    public:
        HafsClientShardFactory(string master_address){
            master_stub_ = Master::NewStub(grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials()));
            cout << "[HafsCLient] Starting Hafs Client Instance!" <<std::endl;
            this->master_address = master_address;

            //check Master status?

            // Get all shards from Master for the first time
            vector<Connection> shards = getShards();

            if (shards.size() == 0) {
                cout << "[HafsCLient] [Error] No shards found in Master" << endl;
            }
            else{
                cout << "[HafsCLient] Found " << shards.size() << " active shards in Master" << endl;
                //Connect client to all shards
                for(int i = 0; i < shards.size(); i++) {
                    shard_servers.push_back(HafsClientFactory(shards[i].primaryaddr(), shards[i].backupaddr()));
                }
            }
            std::thread thread_object(&HafsClientShardFactory::checkHealth, this);
            thread_object.detach();


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
        void balanceLoad()
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

        vector<Connection> getShards() {
            vector<Connection> shards(MAXPOD);
            // Get all shards from Master
            Request request;
            Connection pods;
            ClientContext context;
            std::unique_ptr<ClientReader<Connection> > reader(
                master_stub_->ActiveConnections(&context, request));
            while (reader->Read(&pods)) {
                shards[pods.id()] = pods;
            }
            Status status = reader->Finish();

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
            // cout << "[HafsCLient] Write to shard[" << addr << "]" << endl;
            //get shar_nmbr from addr
            //return shard_servers[shard_nmbr].Write(addr, data);
            return true;
        }

        bool Read(int addr, std::string& data) {
            // cout << "[HafsCLient] Read from shard[" << addr << "]" << endl;
            //get shar_nmbr from addr
            //return shard_servers[shard_nmbr].Read(addr, data);
            return true;
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
        vector<HafsClientFactory> shard_servers;
        bool state;
        
};