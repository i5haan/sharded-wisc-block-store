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
#include "client_impl.h"
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
    public:
        int32_t PbPairCount=0;
        std::string ConfigFilePath;
        std::unordered_map<std::string,Pod> PodList;
        void LoadConfig(string filePath)
        {
            std::fstream file;
            file.open(filePath,std::ios_base::in);

            if(file.is_open())
            {
                std::string temp;
                while(getline(file, temp))
                {
                    std::stringstream sstream(temp);
                    std::string word;
                    int flag = 1;
                    Pod newPod;
                    while(getline(sstream,word,' '))
                    {
                        if(flag==1)
                        {
                            newPod.id = stoi(word);
                            flag=2;
                        }
                        else if(flag==2)
                        {
                            newPod.primaryaddr=word;
                            flag=3;
                        }    
                        else if(flag==3)
                        {
                            newPod.backupaddr=word;
                        }
    
                    }
                    this->PodList[newPod.primaryaddr]=newPod;
                    
                }
                file.close();           
            }
        }
        explicit MasterImpl(std::string FilePath)
        {
            std::cout << "[Master] Starting up the Ha FS server!" << std::endl;
            ConfigFilePath = FilePath;
            LoadConfig(ConfigFilePath);
        }
        Status AckMaster(ServerContext *context, const Connection *req, Response *res) override {
            std::string pAddress = req->primaryaddr();
            std::string sAddress = req->backupaddr();
            Pod newPod;
            if(PodList.find(pAddress)==PodList.end())
            {
                if(PbPairCount+1 == MAXPOD)
                {
                    std::cout << "[Master] MAX shard limit reached" << std::endl;
                    res->set_status(Response_Status_VALID);
                    return Status::OK;
                }

                std::string temp = to_string(PbPairCount+1)+' '+pAddress+' '+sAddress;
                ofstream file;
                ConfigFileLock.lock();
                file.open(ConfigFilePath,std::ios_base::app);
                file<<temp<<endl;
                file.close();
                newPod.primaryaddr = pAddress;
                newPod.id = PbPairCount+1;
                newPod.backupaddr = sAddress;
                PodList[pAddress] = newPod;
                PbPairCount++;
                ConfigFileLock.unlock();

            }
            res->set_status(Response_Status_VALID);
            return Status::OK;
        }
        Status ActiveConnections(ServerContext *context, const Request *req, ServerWriter<Connection> *res) override {
            for(auto it = PodList.begin();it!=PodList.end();it++)
            {
                Pod tempPod = it->second;
                Connection ele;
                ele.set_id(tempPod.id);
                ele.set_primaryaddr(tempPod.primaryaddr);
                ele.set_backupaddr(tempPod.backupaddr);
                res->Write(ele);
            }
            return Status::OK;
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

    HeartBeatResponse_Role roleEnum;

    std::string server_address = serverAddr;
    std::string ConfigFilePath = "/user/avkumar/Avinash/ConfigFile.txt";
    MasterImpl service(ConfigFilePath);
    ServerBuilder builder;
    // HafsClient client(grpc::CreateChannel("0.0.0.0:8091", grpc::InsecureChannelCredentials()), "0.0.0.0:8091", false);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}