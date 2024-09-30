# Docker Build

To build the firmware using docker, follow the following steps:

```
git clone https://github.com/openmv/openmv.git --depth=50
cd openmv/docker
make TARGET=<TARGET NAME>
```

After building you should see the target build output under `docker/build/<TARGET_NAME>`.

## Testing HTTP POST/GET

In order to test HTTP POST and GET request, you can use the following docker image to setup a test web server that can accept POST and GET requests:

```
docker run -v $PWD/images:/tmp/nginx_upload/ -e HOST_UID=$UID -e HOST_GID=$GID --name nginx-requests -d -p 80:80 -p 443:443 openmvcam/nginx-requests:v0.1.0
```

This will run an nginx web server and any image POSTed to the server will be saved within the container in /tmp/nginx_upload and on your host at the <current_directory>/images

## POST Requests

The web server accepts requests to both http and https with following URI `/upload`, and uses the following basic authentication:

```
user: admin
password: testadmin
```

Examples for post requests can be found at `scripts/examples/09-WiFi/http_post.py`.

## GET Requests

The web server accepts requests to both http and https with following URI `/images`.


## Deleting The Server

After finishing testing, you can stop or delete the container simply by running:

```
docker stop nginx-requests
docker rm nginx-requests
```

Note that even after deleting the container you will still find $PWD/images directory on your system.
