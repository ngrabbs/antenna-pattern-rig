FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends python3-uhd python3-numpy python3-serial uhd-host ca-certificates && rm -rf /var/lib/apt/lists/*
RUN uhd_images_downloader -t b2xx
