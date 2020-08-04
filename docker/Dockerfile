FROM ubuntu:16.04

ENV CANOPENSOCKET_SRC https://github.com/CANopenNode/CANopenSocket.git
ENV CANOPENSOCKET_REVISION ee1674c1ca790fc0d5c07a28f4a701d3c929f401
ENV CANOPENSOCKET_DIR /CANopenSocket
ENV APP_DIR /app

RUN apt-get update && apt-get install -y \
    net-tools \
    can-utils \
    git \
    build-essential \
    && apt-get -q autoremove \
    && apt-get -q clean -y \
    && rm -rf /var/lib/apt/lists/* \
    && rm -f /var/cache/apt/*.bin

RUN git clone $CANOPENSOCKET_SRC $CANOPENSOCKET_DIR
WORKDIR $CANOPENSOCKET_DIR
RUN git checkout $CANOPENSOCKET_REVISION
RUN git submodule init; git submodule update

WORKDIR canopend
RUN make

RUN mkdir $APP_DIR
WORKDIR $APP_DIR
ENV PATH="$APP_DIR:${PATH}"

RUN cp $CANOPENSOCKET_DIR/canopend/app/canopend $APP_DIR/canopend
RUN cp $CANOPENSOCKET_DIR/canopend/app/od_storage $APP_DIR/od_storage
RUN cp $CANOPENSOCKET_DIR/canopend/app/od_storage_auto $APP_DIR/od_storage_auto

# Standard canopend port
EXPOSE 6000

CMD ./canopend $CAN -i 1 -c "" -t 6000
