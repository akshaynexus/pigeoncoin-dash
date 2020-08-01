#!/usr/bin/env bash

export LC_ALL=C

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/.. || exit

DOCKER_IMAGE=${DOCKER_IMAGE:-pigeonpay/pigeond-develop}
DOCKER_TAG=${DOCKER_TAG:-latest}

BUILD_DIR=${BUILD_DIR:-.}

rm docker/bin/*
mkdir docker/bin
cp $BUILD_DIR/src/pigeond docker/bin/
cp $BUILD_DIR/src/pigeon-cli docker/bin/
cp $BUILD_DIR/src/pigeon-tx docker/bin/
strip docker/bin/pigeond
strip docker/bin/pigeon-cli
strip docker/bin/pigeon-tx

docker build --pull -t $DOCKER_IMAGE:$DOCKER_TAG -f docker/Dockerfile docker
