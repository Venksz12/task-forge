#!/usr/bin/env sh
set -eu
docker compose -f infrastructure/docker-compose.yml up --build
