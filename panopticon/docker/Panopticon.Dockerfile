FROM python:3.11-slim-bullseye

RUN pip install poetry && \
    apt-get update -y && \
    apt-get install -y build-essential libpq-dev python3-dev procps net-tools

COPY ./pyproject.toml ./poetry.lock ./
RUN poetry install --no-interaction --no-ansi -vvv
COPY ./app.py .

ENTRYPOINT ["poetry", "run", "python", "app.py"]