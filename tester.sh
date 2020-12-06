#!/bin/bash

#Testing

    for i in {1..15}
    do
        ./bin/image-par copy src dest | grep "src/pyramid.bmp"
    done
    echo
    for i in {1..15}
    do
        ./bin/image-par copy src dest | grep "src/balloon.bmp"
    done
    echo
    for i in {1..15}
    do
        ./bin/image-par copy src dest | grep "src/car.bmp"
    done
