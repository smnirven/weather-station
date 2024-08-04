#!/usr/bin/env python
import json, os, sys
from datetime import datetime

import paho.mqtt.client as mqtt
import psycopg2

def get_mqtt_client() -> mqtt.Client:
    mqttc = mqtt.Client()
    mqttc.username_pw_set("admin", "letmein")
    mqttc.connect("rabbitmq", 1883)
    return mqttc

def main():
    topic_name = os.getenv('TOPIC_NAME', 'readings')

    mqttc = get_mqtt_client()

    db_conn = psycopg2.connect("host=timescaledb dbname=postgres user=postgres password=letmein")

    def on_message(client, userdata, msg):
        message = json.loads(msg.payload.decode())
        cur = db_conn.cursor()
        ts = datetime.now()
        cur.execute("INSERT INTO readings (sensor_id, metric_id, time, double) VALUES (%s, %s, %s, %s), (%s, %s, %s, %s)",
            (1, 1, ts, message['internal_temp_c']), (1, 2, ts, message['relative_humidity']))
        # cur.execute("INSERT INTO readings (sensor_id, metric_id, time, double) VALUES (%s, %s, %s, %s)",
        #     (1, 3, ts, message['pressure']))
        db_conn.commit()
        print(" [x] Received %r" % message)

    mqttc.subscribe(topic_name)
    mqttc.on_message = on_message

    print(' [*] Waiting for messages. To exit press CTRL+C')
    
    mqttc.loop_forever()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)