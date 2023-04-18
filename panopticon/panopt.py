from datetime import datetime, timedelta
import json, math, os, itertools, random, time
import typing as t

import click
import pandas as pd
import paho.mqtt.client as mqtt
import sqlalchemy as sql

def get_sql_engine() -> sql.Engine:
    return sql.engine.create_engine(
        f"postgresql://postgres:letmein@timescaledb:5432/postgres"
    )

def get_mqtt_client() -> mqtt.Client:
    mqttc = mqtt.Client()
    mqttc.username_pw_set("admin", "letmein")
    mqttc.connect("rabbitmq", 1883)
    return mqttc

def generate_readings_for_period(start_value: float, end_value: float, times: t.List):
    step = (end_value - start_value) / len(times)
    temps = [start_value] + list([step] * (len(times)-1))
    return list(itertools.accumulate(temps, lambda x, y: x+y))

def live_readings():
    sample_deltas = [-0.25, -0.125, 0, 0.125, 0.25]
    sample_weights = [1, 2, 5, 3, 1]
    topic_name = os.getenv('TOPIC_NAME', 'readings')
    mqttc = get_mqtt_client()
    temp = 9.25
    try:
        while True: 
            time.sleep(5)
            temp = temp + random.choices(sample_deltas, weights=sample_weights, k=1)[0]
            payload = json.dumps({'temp_c': temp, 'timestamp': datetime.now().isoformat()})
            ret = mqttc.publish(topic_name, payload)
            print(" [x] Sent message %s", ret)
    except (KeyboardInterrupt, SystemExit):
        print('\n! Received keyboard interrupt, quitting.\n')
    

@click.command()
@click.option('--mode', type=click.Choice(['backfill', 'live']), default='backfill', help='')
def gen_readings(mode) -> None:
    """Generate realistic fake readings data and insert into the database."""
    if mode == 'live':
        live_readings()
    else:
        backfill_period_mins = 180
        num_periods = 3
        sample_time_secs = 5
        start = datetime.now() - timedelta(minutes=backfill_period_mins)
        p1_mins = backfill_period_mins / num_periods
        p1 = [start + timedelta(seconds=secs) for secs in range(0, math.ceil(p1_mins * 60), sample_time_secs)]
        p2 = [p1[-1] + timedelta(seconds=secs) for secs in range(sample_time_secs, math.ceil(p1_mins * 60), sample_time_secs)]
        p3 = [p2[-1] + timedelta(seconds=secs) for secs in range(sample_time_secs, math.ceil(p1_mins * 60), sample_time_secs)]
        
        engine = get_sql_engine()

        temps = generate_readings_for_period(8.33, 27.78, p1)
        df = pd.DataFrame({'double': temps, 'time':p1, 'sensor_id': list([1] * len(p1)), 'metric_id': list([1] * len(p1))})
        df.to_sql('readings', con=engine, index=False, if_exists='append')

        temps = generate_readings_for_period(27.78, 10.33, p2)
        df = pd.DataFrame({'double': temps, 'time':p2, 'sensor_id': list([1] * len(p2)), 'metric_id': list([1] * len(p2))})
        df.to_sql('readings', con=engine, index=False, if_exists='append')

        temps = generate_readings_for_period(10.33, 32.23, p3)
        df = pd.DataFrame({'double': temps, 'time':p3, 'sensor_id': list([1] * len(p3)), 'metric_id': list([1] * len(p3))})
        df.to_sql('readings', con=engine, index=False, if_exists='append')
    

if __name__ == '__main__':
    gen_readings()