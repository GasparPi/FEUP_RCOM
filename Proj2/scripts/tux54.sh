#!/bin/bash
# this script is meant to be executed at the
# beginning of every RCOM TP class to set up
# each computers ethernet configuration

# activate tux54 ethernet port 0
ifconfig eth0 up

# configure eth0 with address 192.168.50.254 and with 24 bits mask (sub-network 50)
ifconfig eth0 192.168.50.254/24

# echo so we know what's going on
echo "tux54 ethernet port 0 activated with address 192.168.50.254 and with 24 bit mask"

# activate tux54 ethernet port 1
ifconfig eth1 up

# configure eth1 with address 192.168.51.253 and with 24 bits mask (sub-network 51)
ifconfig eth1 192.168.51.253/24

# echo so we know what's going on
echo "tux54 ethernet port 1 activated with address 192.168.51.253 and with 24 bit mask"

# list routes
route -n

# enable forwarding in tux4 (tux1 can commnicate with tux2 and vice-versa in different VLANs)
echo 1 > /proc/sys/net/ipv4/ip_forward
echo "Forwarding has been enabled"

# enbale echo-reply to broadcast request
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
echo "Echo-reply to broadcasr request has been enabled"
