#!/bin/sh

if [ ! -d $1 ]; then
    echo "Not a valid directory: $1"
    exit 1
fi

touch -m $1/opt3002.c
touch -m $1/opt3002.mdef
ln -s -f -T $1 opt3002

