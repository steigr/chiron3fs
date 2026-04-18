FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        autoconf \
        automake \
        build-essential \
        libtool \
        pkg-config \
        libfuse3-dev \
        fuse3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["bash", "-lc", "autoreconf -fi && ./configure && make -j\"$(nproc)\" && make check"]

