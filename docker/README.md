# Docker Build

To build the firmware using docker, follow the following steps:

```
git clone https://github.com/openmv/openmv.git --depth=1
cd openmv/docker
make SDK_DIR=<path/to/openmv-sdk> TARGET=<TARGET NAME>
```

The `SDK_DIR` variable should point to the OpenMV SDK installation directory (defaults to `$HOME/openmv-sdk-<version>`).

After building you should see the target build output under `build/<TARGET_NAME>`.

## Local Development Build

For local iteration there is a parallel `build-firmware-dev` target that mounts the repository (and SDK) at the same absolute path inside the container as on the host. This keeps debug info paths in build artifacts matching the host filesystem, so gdb, addr2line and IDE source navigation work without remapping. It also supports git worktrees and skips the up-front `make clean` so subsequent runs are incremental.

```
cd openmv/docker
make install-sdk                                 # download SDK pinned by SDK_VERSION
make build-firmware-dev TARGET=<TARGET NAME>
make shell-dev                                   # interactive container shell
make clean-dev                                   # wipe the build directory
```

`install-sdk` fetches the version pinned in the repo's `SDK_VERSION` file from `https://download.openmv.io/sdk` and extracts it to `$HOME/openmv-sdk-<version>`. It's idempotent and verifies the sha256. It only needs `wget`, `tar`, and `sha256sum` on the host (no docker required).

When switching `TARGET` between dev builds, run `make clean-dev` first. Per-target build directories are isolated, but `lib/micropython/mpy-cross` is shared and may need to be rebuilt against the new target's headers.

The default `make` target (`build-firmware`) is unchanged and remains the recommended path for reproducible/CI builds.

## Unix Port

The Unix port builds with the image's host compiler, so no SDK is needed. The targets use the same host-path mounts as the local development build. Output goes to `build/UNIX`. See [the Unix port guide](../docs/unix-port.md).

```
cd openmv/docker
make build-unix
make test-unix
make shell-unix
```

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
