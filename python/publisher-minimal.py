#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Source: https://github.com/eclipse/paho.mqtt.python

# Copyright (c) 2014 Roger Light <roger@atchoo.org>
#
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Distribution License v1.0
# which accompanies this distribution.
#
# The Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
#
# Contributors:
#    Roger Light - initial implementation

# This shows an example of using the publish.multiple helper function.

import context  # Ensures paho is in PYTHONPATH

import paho.mqtt.publish as publish

msgs = [{'topic': "hm/room/kitchen/light", 'payload': "on"}, ("hm/room/kitchen/temp", "71", 0, False)]
publish.multiple(msgs, hostname="192.168.0.10")