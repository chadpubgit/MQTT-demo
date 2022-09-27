#!/bin/sh

# Script that detects dnsmasq DHCP lease add events and sends an MQTT message if the MAC address is not listed in a known good list

# Original inspiration
# https://gist.github.com/jwalanta/53f55d03fcf5265938b64ffd361502d5

# INSTALLATION
# Add the following line to the end of /etc/dnsmasq.conf
#
# dhcp-script=/etc/detect_new_device.sh
#
# And then restart services - (maybe just dnsmasq)
#
# service dnsmasq restart 
# service odhcpd restart

# this will be called by dnsmasq everytime a new device is connected
# with the following arguments
# $1 = add | old
# $2 = mac address
# $3 = ip address
# $4 = device name

known_mac_addr_file="/etc/mac-known-systems.csv"
watch_mac_addr_file="/etc/mac-watch-systems.csv"

mqtt_server=192.168.0.10
mqtt_port=1883
mqtt_topic_unknown="hm/alert/med/net"
mqtt_topic_watch="hm/alert/high/net"

#notification_email="1234567890@txt.att.net"

# check if the mac is in known devices list
grep -q "$2" "$known_mac_addr_file"
unknown_mac_addr=$?

if [ "$1" == "add" ] && [ "$unknown_mac_addr" -ne 0 ]; then
  msg="New network device detected on `uci get system.@system[0].hostname`.`uci get dhcp.@dnsmasq[0].domain` $*"
  #echo `date` $msg $unknown_mac_addr >> /tmp/mac-new.log

  mosquitto_pub -h $mqtt_server -p $mqtt_port -t "$mqtt_topic_unknown" -m "$msg"

  # encode colon (:) and send email
  #echo $msg | sed s/:/-/g | sendmail "$notification_email"
fi


# check if the mac is in watch devices list
grep -q "$2" "$watch_mac_addr_file"
watch_mac_addr=$?

if [ "$watch_mac_addr" -eq 0 ]; then
  msg="Watch list network device detected on `uci get system.@system[0].hostname`.`uci get dhcp.@dnsmasq[0].domain` $*"
  #echo `date` $msg $unknown_mac_addr >> /tmp/mac-watch.log

  mosquitto_pub -h $mqtt_server -p $mqtt_port -t "$mqtt_topic_watch" -m "$msg"
fi