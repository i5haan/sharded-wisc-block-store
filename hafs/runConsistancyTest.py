import os

commad_string = "./MultiConsistancyTest"

for i in range(1):
    commad_string+=" & ./MultiConsistancyTest"


commad_string = "./client1 & ./client2"
os.system(commad_string)