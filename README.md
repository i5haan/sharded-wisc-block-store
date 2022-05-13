# SPBBS
For building
```cd hafs```
```mkdir -p cmake/build```
```cd cmake/build```
```pushd cmake/build```
```cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..```
```make -j```
For running server
```./server --port=<PORT> --path=<PATH_TO_BLOCK_DIR> --role=p --address=<other_address>``` For Primary Server
```./server --port=<PORT> --path=<PATH_TO_BLOCK_DIR> --role=b --address=<other_address>``` For Backup Server


For running the basic client
```./basic_read_write``` Note that the address is hardcoded, be aware about that!
 
<br/>

Our code consists of the following main files/components:
- block_manager.h: This file consists of the class/abstraction on how the blocks are persisted inside the block store.
- replicator.h: This file helps in replicating contents of primary to backup.
- master.cc: This file contains the grpc server of the master operations. The calls can be mapped to each of the rpc call in the proto file. It accpets connections from shards and registers them on the shard system. It contains clients to all the other shards as well.
- server.cc: This file consists of the grpc server which serves the block store. It contains replicator and block_manager. It also contains the master client. You can find the code responsible for shuffle in this file as well.
- client_imp.h, clien_lib_main.h, clien_lib.h: These files contains the implementation of the clients. imp is vanilla grpc client for a single shard server. lib is a client combined with both the primary and backup server(or a shard). lib_main is the client containing all such shards as wel as the master.
- We have other supporting files as well, for testing.
- hafs.proto is the protobuf file.

Link to presentation: https://docs.google.com/presentation/d/1Jf3oBXjQ_nHnt3Ni3jcruSzaro_Y4j0TOH662JXc_As/edit#slide=id.g11c21e00768_4_4