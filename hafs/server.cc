#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <mutex>          // std::mutex

#include "hafs.grpc.pb.h"
// #include "client_lib.h"
// #include "block_manager.h"
#include "common.cc"
#include "replicator.h"

using ::Request;
using ::HeartBeatResponse;
using ::ReadRequest;
using ::ReadResponse;
using ::WriteRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::mutex mtx; 

class HafsImpl final : public Hafs::Service {
    private:
        // HafsClient otherMirrorClient;
        HeartBeatResponse_Role role; 
        BlockManager blockManager;
        Replicator replicator;
    public:
        int64_t counter = 0;
        explicit HafsImpl(std::string otherMirrorAddress, HeartBeatResponse_Role role, BlockManager blockManager): replicator(otherMirrorAddress, blockManager) {
            this->role = role;
            this->blockManager = blockManager;
            // this->replicator = replicator;
            std::cout << "Starting up the Ha FS server!" << std::endl;

        }

        Status HeartBeat(ServerContext *context, const Request *req, HeartBeatResponse *res) override {
            res->set_role(this->role);
            return Status::OK;
        }

        Status Read(ServerContext *context, const ReadRequest *req, ReadResponse *res) override {
            std::string data;
            blockManager.read(req->address(), &data);
            res->set_data(data);
            res->set_status(ReadResponse_Status_VALID);
            return Status::OK;
        }

        Status Write(ServerContext *context, const WriteRequest *req, Response *res) override {
            if(!replicator.otherMirrorClient.getIsAlive()) {
                std::cout << "Other Replica down, sending block to replicator after local write!!" << std::endl;
                blockManager.write(req->address(), req->data());
                // Send to replicator!
                res->set_status(Response_Status_VALID);
                return Status::OK;
            } else {
                // std::cout << "Persisting block to other replica!" << std::endl;
                if(replicator.otherMirrorClient.ReplicateBlock(req->address(), req->data())) {
                    blockManager.write(req->address(), req->data());
                    res->set_status(Response_Status_VALID);
                    return Status::OK;
                } else {
                    std::cout << "Write to other replica fail!! Rejecting this write!" << std::endl;
                    res->set_status(Response_Status_INVALID);
                    return Status::OK;
                }
            }
        }

        Status ReplicateBlock(ServerContext *context, const WriteRequest *req, Response *res) override {
            blockManager.write(req->address(), req->data());
            res->set_status(Response_Status_VALID);
            return Status::OK;
        }

};

int main(int argc, char **argv) {
    std::string serverPort;
    std::string fsPath;
    std::string role;
    std::string otherMirrorAddress;

    if(!getArg(argc, argv, "port", &serverPort, 1) || !getArg(argc, argv, "path", &fsPath, 2) || !getArg(argc, argv, "role", &role, 3) || !getArg(argc, argv, "address", &otherMirrorAddress, 4)) {
        exit(1);
    }

    HeartBeatResponse_Role roleEnum;

    if(role.size() == 1 && role[0] == 'p') {
        roleEnum = HeartBeatResponse_Role_PRIMARY;
    } else if(role.size() == 1 && role[0] == 'b') {
        roleEnum = HeartBeatResponse_Role_BACKUP;
    } else {
        std::cout << "Incorrect role specified, can either be 'b' for backup and 'p' for primary!!!" << std::endl;
    }

    std::string server_address = "0.0.0.0:" + serverPort;
    HafsImpl service(otherMirrorAddress, roleEnum, BlockManager(fsPath));
    ServerBuilder builder;
    // HafsClient client(grpc::CreateChannel("0.0.0.0:8091", grpc::InsecureChannelCredentials()), "0.0.0.0:8091", false);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}
