import os

# commad_string = "./MultiConsistancyTest"

# for i in range(1):
#     commad_string+=" & ./MultiConsistancyTest"

commad_string = "./client1 128.105.144.223:8099 128.105.144.211:8100 & ./client2 128.105.144.223:8100 128.105.144.211:8099 & ./client3 128.105.144.223:8101 128.105.144.211:8102 & ./client4 128.105.144.223:8103 128.105.144.211:8104"
os.system(commad_string)