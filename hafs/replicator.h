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
#include <thread>
#include <ctime>
#include <chrono>

#include "client_imp.h"
#include "block_manager.h"

using namespace std;

class Replicator {
    private:
        queue<int> pendingBlocks;
        unordered_map<int, bool> pendinBlocksMap;
        // HafsClient otherMirrorClient;
        BlockManager blockManager;
        std::mutex queueLock;
        HeartBeatResponse_Health health;
    public:
        HafsClient otherMirrorClient;
        Replicator() {

        }
        explicit Replicator(string otherMirrorAddress, BlockManager blockManager): otherMirrorClient(grpc::CreateChannel(otherMirrorAddress, grpc::InsecureChannelCredentials()), otherMirrorAddress, false) {
            this->blockManager = blockManager;
            this->health       = HeartBeatResponse_Health_UNHEALTHY;
            std::thread thread_object(&Replicator::consumer, this);
            thread_object.detach();
        }

        void consumer() {
            while(true) {
                if(!otherMirrorClient.getIsAlive()) {
                    cout << "[Replicator] Other mirror is not alive!! Marking mirror as SINGLE_REPLICA_AHEAD!" << endl;
                    health = HeartBeatResponse_Health_SINGLE_REPLICA_AHEAD;
                    usleep(2*1000000);
                } else if(!pendingBlocks.empty()) {
                    health = HeartBeatResponse_Health_REINTEGRATION_AHEAD;
                    while(!pendingBlocks.empty()) {
                        int nextPendingAddress = removeAndGetLastPendingBlock();
                        cout<< "[Replicator] Processing pending block [" << nextPendingAddress <<  "]" << endl;
                        string data;
                        blockManager.read(nextPendingAddress, &data);
                        if(!otherMirrorClient.ReplicateBlock(nextPendingAddress, data)) {
                            cout<< "[Replicator] Block [" << nextPendingAddress <<  "] failed while trying to replicate, requeueing!" << endl;
                            addPendingBlock(nextPendingAddress);
                        }
                    }
                } else {
                    if(otherMirrorClient.getReplicatorHealth() == HeartBeatResponse_Health_REINTEGRATION_AHEAD){
                        cout << "[Replicator] Pending queue is empty, sleeping for 2 seconds, marking mirror as REINTEGRATION_AHEAD since other mirror has queue" << endl;
                        health = HeartBeatResponse_Health_REINTEGRATION_BEHIND;
                    } else {
                        cout << "[Replicator] Pending queue is empty, sleeping for 2 seconds, marking mirror as HEALTHY" << endl;
                        health = HeartBeatResponse_Health_HEALTHY;
                    }
                    usleep(2*1000000);
                }
            }
        }

        
        void addPendingBlock(int addr) {
            // cout << "adding to queue addr: " << addr << endl;
            queueLock.lock();
            if(pendinBlocksMap.find(addr) == pendinBlocksMap.end()) {
                pendingBlocks.push(addr);
                pendinBlocksMap.insert(make_pair(addr, true));
            }
            queueLock.unlock();
        }

        int removeAndGetLastPendingBlock() {
            queueLock.lock();
            int lastAddr = pendingBlocks.front();
            pendingBlocks.pop();
            pendinBlocksMap.erase(lastAddr);
            queueLock.unlock();

            return lastAddr;
        }

        HeartBeatResponse_Health getHealth() {
            return health;
        }

};