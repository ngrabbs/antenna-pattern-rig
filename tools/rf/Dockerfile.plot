FROM antenna-rig-rf:20261021
RUN apt-get update -qq && apt-get install -y --no-install-recommends python3-matplotlib && rm -rf /var/lib/apt/lists/*
ENV MPLBACKEND=Agg
