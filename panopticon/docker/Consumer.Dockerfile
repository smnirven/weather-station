FROM python:3.11-slim-bullseye

RUN pip install poetry

COPY ./pyproject.toml ./poetry.lock ./
RUN poetry install --only consumer --no-interaction --no-ansi -vvv