#include<bits/stdc++.h>
using namespace std;
#define BLOCK_SIZE 4096
bool LogCommitedBlocks(string fsRoot, int addr)
{
    string CommitedLogPath = fsRoot + "/CommitedLog";
    ofstream file;
    string data = to_string(addr);
    file.open(CommitedLogPath,std::ios_base::app);
    file<<data<<endl;
    file.close();
    return true;
}  

int numShards=2;
int getLogicalAdd(int addr,int n)
{
    int blockID = addr/BLOCK_SIZE;
    int logicalBlockID =  floor(blockID/n);
    return (logicalBlockID*BLOCK_SIZE);
}
bool Write(int addr, std::string data) 
{
    if(addr%BLOCK_SIZE!=0)
    {
        cout<<"UnallignedAddr"<<endl;
    }
    int shardID = (addr/BLOCK_SIZE)%numShards;
    int logicalAddr = getLogicalAdd(addr,numShards);
    if(shardID==0)
    {
        one.Write(logicalAddr,data);
    }
    else if(shardID==1)
    {
        two.Write(logicalAddr,data);
    }
    else
    {
        three.Write(logicalAddr,data);
    }


}

int main()
{
    for(int i=0;i<25;i++)
    {
        int blockno = rand()%2003;
        LogCommitedBlocks("/users/avkumar/Avinash",blockno);
    }

}