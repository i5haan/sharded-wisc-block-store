#include<bits/stdc++.h>
using namespace std;

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

int main()
{
    for(int i=0;i<25;i++)
    {
        int blockno = rand()%2003;
        LogCommitedBlocks("/users/avkumar/Avinash",blockno);
    }

}