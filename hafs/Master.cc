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
#include <sstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <mutex>          // std::mutex
#include <vector>

#include "hafs.grpc.pb.h"
// #include "block_manager.h"
#include "common.cc"
#include "new_client.h"
#include "block_manager.h"

using ::Request;
using ::Connection;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

mutex ConfigFileLock;

#define MAXPOD 10
struct Pod 
{
    int id;
    std::string primaryaddr;
    std::string backupaddr;
};
class MasterImpl final : public Master::Service {
    MasterState_Status status;
    MasterState_Shards shards;

    HafsClientFactory one;
    HafsClientFactory two;
    HafsClientFactory three;
    

    public:
        
        explicit MasterImpl(
            string oneP, string oneB,
            string twoP, string twoB,
            string threeP, string threeB) : one(oneP, oneB), two(twoP, twoB), three(threeP, threeB) {

            shards = MasterState_Shards_TWO;
            status = MasterState_Status_VALID;
            std::cout << "[Master] Starting up the Ha FS server!" << std::endl;
        }

        Status SetState(ServerContext *context, const MasterState *req, Response *res) override {
            status = req->status();
            shards = req->shards();
            return Status::OK;
        }

        Status GetState(ServerContext *context, const Request *req, MasterState *res) override {
            res->set_status(status);
            res->set_shards(shards);
            return Status::OK;
        }
        Status TFCrash(ServerContext *context, const Request *req, Response *res) override {
            std::cout<<"[Testing Master] Crashed After Shuffle Request\n";
            exit(1);
        }


};

int main(int argc, char **argv) {
    std::string serverAddr;
    std::string fsPath;
    std::string role;
    std::string otherMirrorAddress;
    std::string masterAddress;

    if(!getArg(argc, argv, "SAddr", &serverAddr, 1)) {
        exit(1);
    }


    MasterImpl service("10.10.1.1:8093", "10.10.1.1:8094", "10.10.1.1:8095", "10.10.1.1:8096", "10.10.1.1:8097", "10.10.1.1:8098");
    ServerBuilder builder;
    // HafsClient client(grpc::CreateChannel("0.0.0.0:8091", grpc::InsecureChannelCredentials()), "0.0.0.0:8091", false);
    builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAddr << std::endl;
    server->Wait();
    return 0;
}