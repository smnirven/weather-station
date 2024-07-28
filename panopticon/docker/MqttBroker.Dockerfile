FROM rabbitmq:3.11-management
COPY ./docker/rabbitmq/rabbitmq.conf ./docker/rabbitmq/enabled_plugins /etc/rabbitmq/