#!/bin/bash

docker start tpe-arqui-aux
docker exec -it tpe-arqui-aux make clean -C /root/Toolchain
docker exec -it tpe-arqui-aux make clean -C /root/
docker exec -it tpe-arqui-aux make -C /root/Toolchain
docker exec -it tpe-arqui-aux make -C /root/
docker stop tpe-arqui-aux
