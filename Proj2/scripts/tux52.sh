#!/bin/bash
# this script is meant to be executed at the
# beginning of every RCOM TP class to set up
# each computers ethernet configuration

# activate tux51
ifconfig eth0 up

# configure eth0 with address 192.168.51.1 and with 24 bits mask (sub-network 51)
ifconfig eth0 192.168.51.1/24

# echo so we know what's going on
echo "tux52 ethernet port 0 activated with address 192.168.51.1 and with 24 bits mask"

# add route to other subnetwork using a gateway on self subnetwork
# communicate with subnetwork 50 through gateway 253 (tux 4) of subnetwork 51
route add -net 192.168.50.0/24 gw 192.168.51.253
echo "Route for subnetwork 50 has been set up using gatewayt 253"

# list routes
route -n
