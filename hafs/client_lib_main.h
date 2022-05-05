#include "client_lib.h"
#define MAXPOD 10
using namespace std;

#define BLOCK_SIZE 4096

class HafsClientShardFactory {

    public:
        HafsClientShardFactory(string master_address){
            master_stub_ = Master::NewStub(grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials()));
            cout << "[HafsCLient] Starting Hafs Client Instance!" <<std::endl;
            this->master_address = master_address;

            //check Master status?

            // Get all shards from Master for the first time
            vector<Connection> shards = getShards();

            // Connect to all shard servers for the first time
            connectToShards(shards);

        }

        void connectToShards(vector<Connection> shards){
            if (shards.size() == 0) {
                cout << "[HafsCLient] [Error] No shards found in Master" << endl;
            }
            else{
                cout << "[HafsCLient] Found " << shards.size() << " active shards in Master" << endl;
                //Connect client to all shards
                for(int i = 0; i < shards.size(); i++) {
                    if (shard_servers.find(shards[i].id()) == shard_servers.end()) {
                        shard_servers[shards[i].id()] = HafsClientFactory(shards[i].primaryaddr(), shards[i].backupaddr());
                    }
                }
            }

        }

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

            if(status.ok()) {
                cout << "[HafsCLient] Recieved Shards from Master" << std::endl;
            } else {
                cout << "[HafsCLient] Failed to Recieve Shards from Master" << std::endl;
            }

            return shards;
        }

        bool Write(int addr, std::string data) {

            int block1 = addr / BLOCK_SIZE;
            int block2 = (addr % BLOCK_SIZE == 0) ? -1 : block1 + 1;

            int shard_id = getShardNmbrAndLogicalAddr(block1);
            if(block2 != -1) { //unaligned write
                int shard_id2 = getShardNmbrAndLogicalAddr(block2);
                string data2 = data.substr(BLOCK_SIZE - (addr % BLOCK_SIZE), data.size() - (BLOCK_SIZE - (addr % BLOCK_SIZE)));
                data = data.substr(0, BLOCK_SIZE - (addr % BLOCK_SIZE));
            }

            return shard_servers[shard_id].Write(addr, data) && block2 != -1 ? shard_servers[shard_id2].Write(block2 * BLOCK_SIZE, data2) : true;
        }

        bool Read(int addr, std::string& data) {
            int block1 = addr / BLOCK_SIZE;
            int block2 = (addr % BLOCK_SIZE == 0) ? -1 : block1 + 1;

            int shard_id = getShardNmbrAndLogicalAddr(block1);
            if(block2 != -1) { //unaligned read
                int shard_id2 = getShardNmbrAndLogicalAddr(block2);
                string data2;
                string data1;
            }
            if (shard_servers[shard_id].Read(addr, &data1) && block2 != -1 ? shard_servers[shard_id2].Read(block2 * BLOCK_SIZE, &data2) : true) {
                if (block2 != -1) {
                    data = data1 + data2.substr(0, (addr % BLOCK_SIZE));  /* check: server should send BLOCK_SIZE data for second request because it treats it as aligned read?
                                                                             shouldn't be an issue, because only 4k data, otherwise, add another arg for rpc - size? */
                                                                         
                }
                else {
                    data = data1;
                }
                return true;
            }
            return false;
        }

        int  getShardNmbrAndLogicalAddr(int block) {
            cout << "[HafsCLient] Get shard number for block[" << block << "]" << endl;
            return block % shard_servers.size() ;
        }

    private:
        std::unique_ptr<Master::Stub> master_stub_;
        string master_address;
        unordered_map<int, HafsClientFactory> shard_servers;
        
};