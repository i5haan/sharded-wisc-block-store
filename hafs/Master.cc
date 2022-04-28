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

// for streaming
using grpc::ServerWriter;

#define MAXPOD 20
struct Pod 
{
    std::string primaryAddr;
    std::string backupAddr;
};
class MasterImpl final : public Master::Service {
    private:

    // MAXPOD value hardcoded to avoid error: error: expected identifier before numeric constant
    std::vector<Pod> PodList = std::vector<Pod>(MAXPOD);

    // Vector used to stream Connection data to clients.
    std::vector<Connection> ConnectionList = std::vector<Connection>(MAXPOD);

    // PodList2: has an extra variable over PodList which specificies whether the pod is active or not.
    std::vector< pair<Pod, bool> > PodList2 = std::vector< pair<Pod, bool> >(MAXPOD);

    // Declaring the stub
    std::unique_ptr<Hafs::Stub> stub_;
        
    public:

        // Stub implementation done for doing RPC Calls.
        MasterImpl(std::shared_ptr<Channel> channel, std::string address, bool isAlive): stub_(Hafs::NewStub(channel)) {
            std::cout << "[MasterImpl] Starting Master Instance!" <<std::endl;
            this->address = address;
            this->isAlive = isAlive;
            // Get first HearBeat here before starting thread
            checkHeartBeat();
            //std::thread thread_object(&HafsClient::startHeartBeat, this);
            //thread_object.detach();
        }


        int32_t PbPairCount=0;
        int32_t PbPairActiveCount=0;

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

        // Function: SetActiveServer
        // Description:
        // Sets active flag for pods stored in file: "/user/avkumar/Avinash/ConfigFile.txt"
        // Separate DS: PodList2 created for this.
        //
        // Comments: Can be merged with the above function: LoadConfig().
        void SetActiveServer(){

            Pod currPod;

            Request request_p;
            Response response_p;

            Request request_b;
            Response response_b;
            
            ClientContext context_p;
            ClientContext context_b;
            Status status_primary;
            Status status_backup;
            bool PodStatus;

            // Declaring stubs for accessing muliple server pods.
            std::unique_ptr<Hafs::Stub> master_primary_stub_;
            std::unique_ptr<Hafs::Stub> master_backup_stub_;
            
            for(int i = 0; i < PodList.size(); i++)
            {
                // need to put mutex on PodList ?
                currPod = this->PodList[i];

                master_primary_stub_ = Hafs::NewStub(grpc::CreateChannel(currPod.primaryAddr, grpc::InsecureChannelCredentials()));
                master_backup_stub_ = Hafs::NewStub(grpc::CreateChannel(currPod.backupAddr, grpc::InsecureChannelCredentials()));

                status_primary = master_primary_stub_->CheckAlive(&context_p, request_p, &response_p);
                status_backup = master_backup_stub_->CheckAlive(&context_b, request_b, &response_b);
                
                if (!status_primary.ok()) {
                    std::cout << "[Hafs primary] CheckAlive: error code[" << status_primary.error_code() << "]: " << status_primary.error_message() << std::endl;
                    continue;
                    //return false;
                }

                if (!status_backup.ok()) {
                    std::cout << "[Hafs backup] CheckAlive: error code[" << status_backup.error_code() << "]: " << status_backup.error_message() << std::endl;
                    continue;
                    //return false;
                }

                if((response_p.status() == Response_Status_VALID) && (response_b.status() == Response_Status_VALID)){
                    PodStatus = true;
                }
                else
                {
                    PodStatus = false;
                }
                
                this->PodList2[i] = make_pair(currPod, PodStatus);
            }
        }


        // Function: ActiveConnections()
        // Description:
        //
        // RPC used by client to request stream of Connection data (which contains address of primary, backup and active-status of pods)
        Status ActiveConnections(ServerContext *context, const Request *request, ServerWriter<Connection>* writer) override {
            
            // std::string pri_address;
            // std::string bac_address;

            // setting ConnectionList, need mutex to access PodList2
            for(int i = 0; i < this->PodList2.size(); i++){
                
                ConnectionList[i].set_id(this->PodList2[i].second);
                ConnectionList[i].set_primaryaddr((this->PodList2[i]).first.primaryAddr);
                ConnectionList[i].set_backupaddr((this->PodList2[i]).first.backupAddr);
            }

            for (const Connection& f : ConnectionList) {
                
                //pri_address = f.primaryAddr;
                //bac_address = f.backupAddr

                if(f.id() == 1){
                    writer->Write(f);
                }
                
            }
            return Status::OK;

        }
};
