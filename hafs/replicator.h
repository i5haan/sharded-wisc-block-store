#include<iostream>
#include <queue>
#include <unordered_map>
#include <grpc/grpc.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "client_lib.h"
#include "block_manager.h"

using namespace std;

class Replicator {
    private:
        queue<int> pendingBlocks;
        unordered_map<int, bool> pendinBlocksMap;
        // HafsClient otherMirrorClient;
        BlockManager blockManager;
    public:
        HafsClient otherMirrorClient;
        Replicator() {

        }
        explicit Replicator(string otherMirrorAddress, BlockManager blockManager): otherMirrorClient(grpc::CreateChannel(otherMirrorAddress, grpc::InsecureChannelCredentials()), otherMirrorAddress, false) {
            this->blockManager = blockManager;
        }

        
        void addPendingBlock(int addr) {
            if(pendinBlocksMap.find(addr) != pendinBlocksMap.end()) {
                pendingBlocks.push(addr);
                pendinBlocksMap.insert(make_pair(addr, true));
            }
        }

        int removeAndGetLastPendingBlock() {
            int lastAddr = pendingBlocks.front();
            pendingBlocks.pop();
            pendinBlocksMap.erase(lastAddr);

            return lastAddr;
        }

};