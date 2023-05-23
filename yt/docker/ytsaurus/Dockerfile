FROM ubuntu:focal

USER root

WORKDIR /tmp

ARG SPYT_VERSION=1.69.3
ARG SPARK_VERSION=1.69.0

# Main binary.
COPY ./ytserver-all /usr/bin/ytserver-all

# If this list changes, also update yt_nightly/Dockerfile
RUN ln /usr/bin/ytserver-all /usr/bin/ytserver-master && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-clock && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-discovery && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-node && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-job-proxy && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-exec && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-proxy && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-http-proxy && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-tools && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-scheduler && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-controller-agent && \
    ln /usr/bin/ytserver-all /usr/bin/ytserver-query-tracker

COPY ./init_operation_archive.py /usr/bin/init_operation_archive

# CHYT bits and pieces.
COPY ./chyt-controller /usr/bin/chyt-controller
COPY ./clickhouse-trampoline.py /usr/bin/clickhouse-trampoline
COPY ./ytserver-clickhouse /usr/bin/ytserver-clickhouse
COPY ./ytserver-log-tailer /usr/bin/ytserver-log-tailer

# SPYT pieces.
COPY ./spyt_release/home/spark/conf/releases/${SPARK_VERSION}/metrics.properties /usr/bin/metrics.properties
COPY ./spyt_release/home/spark/conf/releases/${SPARK_VERSION}/solomon-agent.template.conf /usr/bin/solomon-agent.template.conf
COPY ./spyt_release/home/spark/conf/releases/${SPARK_VERSION}/solomon-service-master.template.conf /usr/bin/solomon-service-master.template.conf
COPY ./spyt_release/home/spark/conf/releases/${SPARK_VERSION}/solomon-service-worker.template.conf /usr/bin/solomon-service-worker.template.conf
COPY ./spyt_release/home/spark/spark/releases/${SPARK_VERSION}/spark.tgz /usr/bin/spark.tgz
COPY ./spyt_release/home/spark/spyt/releases/${SPYT_VERSION}/spark-yt-data-source.jar /usr/bin/spark-yt-data-source.jar
COPY ./spyt_release/home/spark/spyt/releases/${SPYT_VERSION}/spyt.zip /usr/bin/spyt.zip
COPY ./spyt_release/home/spark/bin/releases/${SPARK_VERSION}/spark-yt-launcher.jar /usr/bin/spark-yt-launcher.jar

# CREDITS files with licenses for all binaries.
COPY ./credits/ytserver-all.CREDITS /usr/bin/ytserver-all.CREDITS
COPY ./credits/chyt-controller.CREDITS  /usr/bin/chyt-controller.CREDITS
COPY ./credits/ytserver-clickhouse.CREDITS /usr/bin/ytserver-clickhouse.CREDITS
COPY ./credits/ytserver-log-tailer.CREDITS  /usr/bin/ytserver-log-tailer.CREDITS

RUN apt-get update && apt-get install -y \
  curl \
  less \
  gdb \
  lsof \
  strace \
  telnet \
  dnsutils \
  iputils-ping \
  lsb-release \
  openjdk-11-jdk \
  python3.10

RUN pip install \
    ytsaurus-client \
    ytsaurus-yson \
    ytsaurus-local \
    ytsaurus-spyt==${SPYT_VERSION}

# Local built package with native driver bindings.
RUN pip install ./ytsaurus_native_driver-1.0.0-cp34-abi3-linux_x86_64.whl

RUN ln -s /usr/lib/jvm/java-11-openjdk-amd64 /opt/jdk11

# TODO: add opportunity to get this files in opensource
# Static binaries & shared libraries built in arcadia.
# In order to obtain them, run the following command in yt/yql/package:
#   ya package package.json --raw-package-path yql_package
# And move the resulting directory to the context of docker build.
COPY ./yql_package/ytserver-yql-agent /usr/bin/
COPY ./yql_package/mrjob /usr/bin/
COPY ./yql_package/lib*.so /usr/lib/yql/
