#include "client_lib.h"
#include<bits/stdc++.h>
#define MAXPOD 10
using namespace std;

using ::Connection;
using ::Response;
using ::Request;
using ::MStatus;

#define BLOCK_SIZE 4096

class StaticThreeShardFactory {
    int numShards = 2; // 2 or 3
    HafsClientFactory one;
    HafsClientFactory two;
    HafsClientFactory three;

    public:
        StaticThreeShardFactory(
            string oneP, string oneB,
            string twoP, string twoB,
            string threeP, string threeB) : one(oneP, oneB), two(twoP, twoB), three(threeP, threeB) {

        }

        int getShards() {
            return numShards;
        }

        void setShardFromMaster() {
            //  
        }

        // void write(int blockNum) {
        //     int shardId = blockNum % numShards;

        //     if(shardId == 0) {
        //         one.Write();
        //     } else if(shardId == 1) {
        //         two.Write();
        //     } else if(shardId == 2) {
        //         three.Write();
        //     }
            

        // }
};