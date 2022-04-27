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

#define MAXPOD 10
struct Pod 
{
    std::string primaryAddr;
    std::string backupAddr;
};
class MasterImpl final : public Master::Service {
    private:
    std::vector<Pod> PodList(MAXPOD);
    public:
        int32_t PbPairCount=0;
        int32_t PbPairActiveCount=0;
        HafsClient 
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
                            newPod.primaryAddr=word;
                            flag=2;
                        }    
                        else if(flag==2)
                        {
                            newPod.backupAddr=word;
                        }
    
                    }
                    this->PodList[PbPairCount++]=newPod;
                    
                }
                file.close();           
            }
        }
        explicit MasterImpl()
        {
            std::cout << "[Server] Starting up the Ha FS server!" << std::endl;
            std::string ConfigFilePath = "/user/avkumar/Avinash/ConfigFile.txt"
            LoadConfig(ConfigFilePath);
        }

}
