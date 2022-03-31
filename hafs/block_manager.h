#include<iostream>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>
#include <unordered_map>
#include <ios>
#include <cstdio>
#include <fstream>
#include <utility>

using namespace std;

const int BLOCK_SIZE         = 4096;
const int BLOCK_DIVIDER_SIZE = 2048;
const int BLOCK_PER_DIVIDER  = 2048;
const int MAX_BLOCK          = BLOCK_PER_DIVIDER*BLOCK_DIVIDER_SIZE;


class BlockManager {
    private:
        string fsRoot;

        bool isAlligned(int addr) {
            if(addr % BLOCK_SIZE == 0) {
                return true;
            }
            return false;
        }

        int getBlockNumber(int addr) {
            return addr / BLOCK_SIZE;
        }

        int getBlockOffset(int blockNumber) {
            return blockNumber % BLOCK_DIVIDER_SIZE;
        }
        
    public:
        BlockManager() {

        }
        BlockManager(string fsRoot) {
            this->fsRoot = fsRoot;
        }

        // vector<int

        bool write(int addr, string data) {
            if(!isAlligned(addr)) {
                return unallignedWrite(addr, data);
            }

            return allignedWrite(addr, data);
        }

        bool allignedWrite(int addr, string data) {
            int blockNumber = getBlockNumber(addr);
            int blockOffset = getBlockOffset(blockNumber);
            string offsetPath = fsRoot + "/" + to_string(blockOffset);
            string blockPath = offsetPath + "/" + to_string(blockNumber);
            cout << "[BlockManager] Performing an alligned write on addr[" << addr <<"] with block path[" << blockPath << "] and offset path[" << offsetPath << "]" << endl;
            
            mkdir(offsetPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO); // To make sure the offset directory exists

            int fd = open(blockPath.c_str(),
                O_CREAT | O_RDWR,
                S_IRWXU | S_IRWXG | S_IRWXO);

            int writeRes = pwrite(fd, &data[0], data.size(), 0);
            
            if(writeRes == -1) {
                close(fd);
                return false;
            }

            close(fd);
            return true;
        }

        bool unallignedWrite(int addr, string data) {
            return true;
        }

        bool read(int addr, string* data) {
            if(!isAlligned(addr)) {
                return unallignedRead(addr, data);
            }
            return allignedRead(addr, data);
        }   

        bool allignedRead(int addr, string* data) {
            int blockNumber = getBlockNumber(addr);
            int blockOffset = getBlockOffset(blockNumber);
            string offsetPath = fsRoot + "/" + to_string(blockOffset);
            string blockPath = offsetPath + "/" + to_string(blockNumber);
            cout << "[BlockManager] Performing an alligned read on addr[" << addr <<"] with block path[" << blockPath << "] and offset path[" << offsetPath << "]" << endl;
            

            int fd = open(blockPath.c_str(),
                O_RDONLY,
                S_IRWXU | S_IRWXG | S_IRWXO);

            if(fd < 0) {
                return false; // Probably this block does not exist!
            }

            string buf;
            buf.resize(BLOCK_SIZE);
            int readRes = pread(fd, &buf[0], BLOCK_SIZE, 0);
            data->resize(BLOCK_SIZE);
            data->replace(0, BLOCK_SIZE, buf);
            
            if(readRes == -1) {
                close(fd);
                return false;
            }

            close(fd);
            return true;
        }

        bool unallignedRead(int addr, string* data) {
            return true;
        }
        
};