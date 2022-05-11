import os

commad_string = "./MultiConsistancyTest"

for i in range(1):
    commad_string+=" & ./MultiConsistancyTest"


    //Shard_ClientConsistencyDiffAddr( 10000, "128.105.144.230:8093","c", true);
commad_string = "./client1 128.105.144.223:8093 128.105.144.211:8094 & ./client2 128.105.144.223:8093 128.105.144.211:8094"
os.system(commad_string)