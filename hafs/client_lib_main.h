#include "client_lib.h"

using namespace std;

using ::Connection;

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
                    shard_servers.push_back(HafsClientFactory(shards[i].primaryAddr(), shards[i].backupAddr()));
                }
            }

        }

        vector<Connection> getShards() {
            vector<Connection> shards;
            // Get all shards from Master
            Request request;
            Connection pods;

            std::unique_ptr<ClientReader<Connection> > reader(
                master_stub_->ActiveConnections(&context, request));
            while (reader->Read(&pods)) {
                shards.push_back(pods);
            }
            Status status = reader->Finish();

            if(status.ok()) {
                cout << "[HafsCLient] Recieved Shards from Master" << std::endl;
            } else {
                cout << "[HafsCLient] Failed to Recieve Shards from Master" << std::endl;
            }

            return shards;
        }


        bool Write(int addr, std::string data) {
            // cout << "[HafsCLient] Write to shard[" << addr << "]" << endl;
            //get shar_nmbr from addr
            return shard_servers[shard_nmbr].Write(addr, data);
        }

        bool Read(int addr, std::string& data) {
            // cout << "[HafsCLient] Read from shard[" << addr << "]" << endl;
            //get shar_nmbr from addr
            return shard_servers[shard_nmbr].Read(addr, data);
        }

    private:
        std::unique_ptr<Master::Stub> master_stub_;
        string master_address;
        vector<HafsClientFactory> shard_servers;
        
};