#include<iostream>
#include<thread>
#include<chrono>
#include "new_client.h"

using namespace std;



int main() {
    StaticThreeShardFactory client("10.10.1.1:8093", "10.10.1.1:8094", "10.10.1.1:8095", "10.10.1.1:8096", "x", "x");

    while(true);

    return 0;
}