#!/bin/bash
# this script is meant to be executed at the
# beginning of every RCOM TP class to configure
# each computers ethernet configuration

# activate tux51
ifconfig eth0 up

# configure eth0 with address 192.168.50.254 and with 24 bits mask
ifconfig eth0 192.168.50.254/24

# configure eth1 with address 192.168.51.253 and with 24 bits mask
ifconfig eth1 192.168.51.253/24
