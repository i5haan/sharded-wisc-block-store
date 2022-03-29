
#include <grpc/grpc.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <iostream>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <thread>

#include "rnbss.grpc.pb.h"

using ::Request;
using ::Response;
using ::ReadRequest;
using ::DataBlock;
using ::WriteRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

#define BLOCKSIZE 4096

std::string filePath = "/tmp/test.txt";

class RnbssServer final : public Rnbss::Service {
    public:
        explicit RnbssServer() 
        {
            std::cout << "Starting up the RNBSS service!" << endl;
        }
        Status isAlive(ServerContext *context, const Request* DB, Response *response)
        {
            std::cout<<"[server] Status Check " << std::endl;
            response.set_status(1);

        }
        Status Write(ServerContext *context, const WriteRequest* DB, Response *response)
        {
            std::cout<<"[server] Writing the file " << filePath << std::endl;
            int fd;
            fd = open(filePath.c_str(), O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
            if(fd < 0) {
                cout<<"error: cant open file to write\n";
                return -1;
            }
            std::string buf = DB.data();
            int writeData = pwrite(fd, &buf[0], buf.size(), DB.address());
            if(writeData==-1)
            {
                cout<<"error: write failed\n";
            } 
            else
                response.set_status(1);            
            close(fd);
            return Status::OK;
        }
        Status Read(ServerContext *context, const ReadRequest* request, DataBlock *DB)
        {
            int fd;
            fd = open(filePath.c_str(), O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
            if(fd < 0) {
                cout<<"error: cant open file to write\n";
                return -1;
            } 
            std::string buf;
            int readData = pread(fd, &buf[0], BLOCKSIZE, request.address());
            if(readData==-1)
            {
                cout<<"error: No data to read failed\n";
            } 
            else
            {
                DB->set_data(buf);
                DB->set_status(1);
            }           
            close(fd);
            return Status::OK;
        }

}

int main(int argc, char **argv) {
    std::string server_address("address");
    RnbssServer service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}
 