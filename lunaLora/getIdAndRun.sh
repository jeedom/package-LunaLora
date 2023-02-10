#!/bin/sh

GATEWAYID=$(util_chip_id/chip_id -d /dev/spidev32766.0 | grep -io "concentrator EUI: 0x*[0-9a-fA-F][0-9a-fA-F]*\+" | awk '{print substr($NF,-16)}'| tr -d 'x')

sed -i "s|xxxxx|$GATEWAYID|" global_conf.json

packet_forwarder/lora_pkt_fwd

