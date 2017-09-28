#!/bin/bash
exec make clean release CC=i486-linux-musl-gcc STRIP=i486-linux-musl-strip CFLAGS=-static
