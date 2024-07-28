CREATE TABLE sensors(
    id SERIAL PRIMARY KEY,
    type VARCHAR(50),
    location VARCHAR(50)
);

CREATE TYPE MetricDataType AS ENUM ('double', 'integer');

CREATE TABLE metrics (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    data_type MetricDataType NOT NULL,
    units VARCHAR(50) NOT NULL
);

CREATE TABLE readings (
    time TIMESTAMPTZ NOT NULL,
    sensor_id INTEGER,
    metric_id INTEGER,
    double DOUBLE PRECISION,
    integer INTEGER,
    FOREIGN KEY (sensor_id) REFERENCES sensors (id),
    FOREIGN KEY (metric_id) REFERENCES metrics (id)
);

SELECT create_hypertable('readings', 'time');

CREATE INDEX readings_sensors_idx ON readings (sensor_id, time desc);
CREATE INDEX readings_sensors_metrics_idx ON readings (sensor_id, metric_id, time desc);

INSERT INTO sensors (id, type, location) VALUES (1, 'temperature', 'weather_station');
INSERT INTO metrics (id, name, data_type, units) VALUES (1, 'temperature', 'double', 'degrees_celcius');

