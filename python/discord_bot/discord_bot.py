#!/usr/bin/python3
"""

TODO
- add a list of channels to subscribe to

----

Example Discord bot that relays MQTT messages into a private Discord channel


Creating a Discord bot Instructions: https://discordpy.readthedocs.io/en/stable/
Updates to instructions:
- After getting the bot's token, make sure to enable 'MESSAGE CONTENT INTENT' in the bot's Privileged Gateway Intents
- On the OAuth2 tab, select the following
-- Scope: bot
-- Bot permissions: Send Messages


To use this bot, create a config.py file in the same directory as this file.
Paste the following text into config.py and then add your information.


class Config(object):

    # DISCORD_TOKEN is the Discord bot's token
    DISCORD_TOKEN = 'INSERT-TOKEN-HERE'

    # DISCORD_MAIN_CHANNEL is the channel ID to send messages to
    # - To obtain a channel ID, In the Discord app right click the channel and click 'Copy ID'
    DISCORD_MAIN_CHANNEL = 123456789012345678

    # MQTT_BROKER_ADDRESS is your MQTT brokers IP address or hostname
    MQTT_BROKER_ADDRESS = '127.0.0.1'

    # MQTT_BROKER_PORT is your MQTT broker's port number
    MQTT_BROKER_PORT = 1883

"""

import asyncio
import logging
import time

import discord

from amqtt.client import MQTTClient
from amqtt.mqtt.constants import QOS_1
from config import Config


# constants
TOPICS = [('hm/#', QOS_1)]
TEST_TOPIC = 'hm/test/discord'

# globals
intents = discord.Intents.default()
intents.message_content = True
client = discord.Client(intents=intents)
logger = logging.getLogger(__name__)
mqtt = None


# Respond to discord text messages (if you do not need this feature, remove this function and 'intents.message_content = True' listed above)
@client.event
async def on_message(message):
    # we do not want the bot to reply to itself
    if message.author == client.user:
        return

    if message.content.startswith('!hello'):
        msg = 'Hello {0.author.mention}'.format(message)
        await message.channel.send(msg)

    # Generate and MQTT system message
    if message.content.startswith('!mqtt'):
        if mqtt == None:
            await message.channel.send('MQTT is down')
        else:
            testMsg = "Discord bot MQTT test (" + str(int(time.time())) + ")"
            try:
                await mqtt.publish(TEST_TOPIC, testMsg.encode(), qos=0x01)
                await message.channel.send('MQTT test message sent')
            except Exception as e:
                await message.channel.send('MQTT test message publish failed: {}'.format(e))
                logger.error('publish failed: {}'.format(e))

@client.event
async def on_ready():
    logging.info('Logged in as {} - id: {}'.format(client.user.name, client.user.id))
    channel = client.get_channel(Config.DISCORD_MAIN_CHANNEL)
    await channel.send('I am ready')

async def post_message_to_channel(msg):
    logging.info('Posting message to channel: {}'.format(msg))
    try:
        channel = client.get_channel(Config.DISCORD_MAIN_CHANNEL)
        await channel.send('{} -- {}'.format(time.strftime('%m/%d %H:%M:%S'), msg))
    except Exception as e:
        logging.info('Error posting to channel: {}'.format(e))

async def discord_task():
    while True:
        logging.info('starting Discord task')
        #await client.run(Config.DISCORD_TOKEN)    # don't use client.run as it abstracts asyncio loop

        try:
            await client.start(Config.DISCORD_TOKEN)
        except Exception as e:
            logging.error('Exception: {}'.format(e))            
            logging.exception('Caught an error')
            if str(e).find('requesting privileged intents'):
                logging.warn("Discord bot may not have the correct privileged intents. Verify that the bot has the MESSAGE CONTENT INTENT enabled.")

        logging.info('STOPPING Discord task')

async def mqtt_task():
    global mqtt

    logging.info('starting MQTT')
    mqtt = MQTTClient(config={'reconnect_retries': -1}) # reconnect_retries allows unlimited reconnects
    await mqtt.connect('mqtt://{}:{}/'.format(Config.MQTT_BROKER_ADDRESS, Config.MQTT_BROKER_PORT), cleansession=False) # cleansession False re-subscribes to topics
    await mqtt.subscribe(TOPICS)
    logger.info("Subscribed")
    while True:
        try:
            message = await mqtt.deliver_message()
            packet = message.publish_packet
            logging.info("mqtt: %s => %s" % (packet.variable_header.topic_name, str(packet.payload.data)))        
            msg = packet.payload.data.decode()
            #logging.info('msg format: {}'.format(repr(msg)))
            await post_message_to_channel(msg)
        except Exception as e:
            logging.info('Error decoding MQTT message: {}'.format(e))

    # await mqtt.unsubscribe([x[0] for x in TOPICS])
    # logger.info("UnSubscribed")
    # await mqtt.disconnect()

    #logging.info('STOPPING MQTT task')


if __name__ == '__main__':
    #formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    #formatter = "[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    formatter = "[%(asctime)s] %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)

    loop = asyncio.get_event_loop()
    loop.create_task(discord_task())
    loop.create_task(mqtt_task())
    loop.run_forever()

    logging.info('[DONE]')
