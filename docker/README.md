# CANopen gateway in Docker

The dockerfile creates a CANopen master with gateway functionality.
Using the commands interface one can control a CANopen network.
The gateway is exposed at port *6000*.

## Dependencies

This project is based upon [CANopenSocket](https://github.com/CANopenNode/CANopenSocket) and [CANopenNode](https://github.com/CANopenNode/CANopenNode).

## Building the container image

This project should build with the default Docker build engine, but also *buildx* is supported for creating cross-platform container images.
Considering you're targetting embedded devices I'd guess you're cross building, so let's demonstrate *docker buildx* here.

1. Make sure that Docker buildx is installed and that the Docker experimental features are enabled in your environment.
2. Either create a new or use a already created buildx engine:

```bash
$ docker buildx create --name crossbuilder
$ docker buildx use crossbuilder
```

3. When building, we also want to push the build output to Docker hub so make sure this is also setup correctly and that're logging in:

```bash
$ docker login
```

4. Now build (and push) the container image:

```bash
$ docker buildx build --platform linux/arm,linux/arm64,linux/amd64 -t geoffreyvl/canopensocket . --push
```

5. On Docker Hub your container image will appear
6. We can now use the container image, for example let's inspect it:

```bash
$ docker buildx imagetools inspect geoffreyvl/canopensocket
Name:      docker.io/geoffreyvl/canopensocket:latest
MediaType: application/vnd.docker.distribution.manifest.list.v2+json
Digest:    sha256:bd24fa61e43427f1113027e299b63c455510bb5d0fe9e2600d7db3fdde7ea58a
        
Manifests: 
Name:      docker.io/geoffreyvl/canopensocket:latest@sha256:8d953afb62cd220e44c45abb0247c6ce6726b43bfe2b825475672e14e1996684
MediaType: application/vnd.docker.distribution.manifest.v2+json
Platform:  linux/arm/v7
        
Name:      docker.io/geoffreyvl/canopensocket:latest@sha256:388e652c8e7796f6746fa45afedbb0c1fd8d2ad29d28d4c3c6228084b549b98c
MediaType: application/vnd.docker.distribution.manifest.v2+json
Platform:  linux/arm64
        
Name:      docker.io/geoffreyvl/canopensocket:latest@sha256:6bfeb074195e93e4f4774bc08f9539c6f4f34042e124805485af0930d51e4ceb
MediaType: application/vnd.docker.distribution.manifest.v2+json
Platform:  linux/amd64
```

## Using the container image

### Setup CAN

On your native host machine make sure that your CAN interface is up and running.
We will use that interface within the Docker container.

```bash
$ ip link set can0 type can bitrate 250000
$ ip link set can0 up
```

If you don't own a can interface can create a virtual can device:

```bash
$ sudo modprobe vcan
$ sudo ip link add dev vcan0 type vcan
$ sudo ip link set up vcan0
```

Verify using ifconfig:

```bash
$ ifconfig
vcan0: flags=193<UP,RUNNING,NOARP>  mtu 72
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

### Running a container

Either you build your own Docker image using the provided Dockerfile, or you use a [3rd party](https://hub.docker.com/repository/docker/geoffreyvl/canopensocket) provided Docker image.
Following guidelines assume the latter.

To run the container software you must pass the can interface into the container using the `-e CAN=<if>` option.
To run the process in background/detached mode pass the `-d` option.
Run the container as following:

```bash
$ docker run --rm --network host -d -e CAN=can0 "geoffreyvl/canopensocket:latest"
```

Verify that the container is running:

```bash
$ docker container ls
```

## Interact with the CANopen gateway

On your host system make sure to have the `canopencomm` application.
It's part of the [CANopenSocket](https://github.com/CANopenNode/CANopenSocket) sources, but you need to compile it from source.
To send commands through your gateway, use syntax according to the example below:

```bash
$ canopencomm -t localhost -p 6000
```

For more examples visit the [CANopenSocket](https://github.com/CANopenNode/CANopenSocket) repo.

## Monitoring

You  can use a second terminal to dump the traffic at your can0 interface:

```bash
$ candump can0
...
 can0  701   [1]  7F
 can0  000   [2]  01 01
 can0  181   [2]  00 00
 can0  701   [1]  05
```
