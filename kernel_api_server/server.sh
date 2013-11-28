#/bin/bash
g++ -I. -I$ICE_HOME/include  -lpthread  -c ../kernel_ice/Ipmstor.cpp server.cpp netmask.cpp errors.cpp 
g++  -o server Ipmstor.o server.o errors.o netmask.o  -L$ICE_HOME/libg -lIce -lIceUtil  -lmysqlclient -I/usr/include/mysql/ -L/usr/lib64/mysql ./IPMS_config_so/config.so 
