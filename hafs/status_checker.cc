#include<iostream>
#include "client_lib.h"

using namespace std;


int main() {
    HafsClientFactory client("0.0.0.0:8090", "0.0.0.0:8091");
    while(true);
    return 0;
}