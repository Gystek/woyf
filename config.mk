# woyf version
VERSION=0.1.0

# Customize below to fit your system

# paths
PREFIX=/usr/local/
MANPREFIX=$(PREFIX)/share/man

# flags
CFLAGS=-ansi -Wall -Wextra -Wpedantic -O2 -g -D_POSIX_C_SOURCE
LDFLAGS=

# compiler and linker
CC=gcc
LD=$(CC)
