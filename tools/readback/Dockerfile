FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends avrdude python3 python3-serial binutils-avr && rm -rf /var/lib/apt/lists/*
WORKDIR /backup
