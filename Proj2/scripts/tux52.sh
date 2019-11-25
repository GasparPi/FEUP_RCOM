#!/bin/bash
# this script is meant to be executed at the
# beginning of every RCOM TP class to set up
# each computers ethernet configuration

# activate tux51
ifconfig eth0 up

# configure eth0 with address 192.168.51.1 and with 24 bits mask (sub-network 51)
ifconfig eth0 192.168.51.1/24
