#include <iostream>
#include "SHA256.h"
 
using std::string;
using std::cout;
using std::endl;
 
int main(int argc, char *argv[])
{
    string A = "grape";
	string B = "grapes";
    //string output1 = sha256(A);
	//string output1 = sha256(A);
	if(sha256(A)==sha256(B))
	cout<<"true"<<endl;
	else
	cout<<"False"<<endl;
    
    return 0;
}