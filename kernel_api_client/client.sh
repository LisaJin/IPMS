#/bin/bash
g++  -I. -I$ICE_HOME/include -c ../kernel_ice/Ipmstor.cpp client.cpp 
g++   -o client Ipmstor.o client.o   ../kernel_api_server/IPMS_config_so/config.so -L$ICE_HOME/lib -lIce -lIceUtil 
 


