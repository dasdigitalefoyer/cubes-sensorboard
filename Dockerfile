FROM python:slim-buster

RUN pip install -U PlatformIO
WORKDIR /puzzleCubes
ADD . ./modules/pc-sensorboard

WORKDIR /puzzleCubes/modules/pc-sensorboard

ARG VERSION=6.1.11

# RUN pio pkg install
# RUN pio run
# RUN pio pkg uninstall  -g


CMD [ "pio", "run", "-t", "upload" ]