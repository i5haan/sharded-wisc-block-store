#include<iostream>
#include "client_lib.h"
#include "common.cc"

using namespace std;

int main(int argc, char **argv) {
//    HafsClientFactory client("0.0.0.0:8080", "0.0.0.0:8081");
      HafsClientFactory client("server-0.test1.uwmadison744-f21.emulab.net:50052", 
    "server-1.test1.uwmadison744-f21.emulab.net:50053");
 
    std::string testCase;
    if(!getArg(argc, argv, "tc", &testCase, 1)){
        exit(1);
    }
    std::cout << "testCase " << testCase << endl;
    string res;

    //1. Test case : primary receives write request and fails 
    // output : not persisted on either on primary & backup
        if(testCase == "primaryFail"){
            client.Write(4096, string(4096, 'a'));  
            // diff in python file
        }
        
        if(testCase == "clientRetryRequired") {
            // clientRequiredNow -> won't be required post temp file creation
            //2. Test case : primary receive write request, backup writes it and send ack to primary
            // and then while writing to primary it fails
            // output : Unless client is retrying (since it didn't receive ack), inconsistency present
            
            client.Write(8192, string(4096, 'b'));
        // usleep(3*1000000); // 10 seconds
            // // assumption : failover would open, now backup is primary and serving your request 
            // and pending queue of primary contains this write request 
        // client.Write(8192, string(4096, 'b'));
            // start primary
            // check consistency after this.
        }
        
        // primary fails after writing 
        // (that means client doesn't received ack, but both primary and backup is having data)
    //  client.Write(12288, string(4096,'c'));
    // retry
    //client.Write(12288, string(4096,'c'));



        // ******* KILL BACKUP SERVER MANUALLY *****    
        // queue behaviour 
        // primary is up and backup is down.
        // demo better? write shell script?

        // ******* BRING UP BACKUP SERVER *****    
        // until healhty, don't accept -> replicator state
        

        // USPs of our system -> replicator state 

}