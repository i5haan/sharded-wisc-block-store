#include "ConsistancyTest.h"

using namespace std;

int main() {
    
    int TC = 2;
    int k=0;
    int code = SingleClientConsistencyDiffAddr();
    if(code)
        k++;
    code = SingleClientConsistencySameAddr(24);
    if(code)
        k++;
    if(k!=TC)
        cout<<"Cosistancy failed\n";
    else
        cout<<"Success"<<endl;
    /*for(int i=1;i<=1;i++)
    {
        cout<<"No of Client = "<<i*4<<endl;
        int code = MultiClientConsistencySameAddr(i*4);
        if(code==0)
            cout<<"MultiThread Consistancy failed\n";
        else
            cout<<"MultiThread Consistancy Success"<<endl;
    }*/
    return 0;
}