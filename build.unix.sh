#!/bin/sh
cc -o bin/netelf.`uname -s`.`uname -m`.exe netelf.c
cc -o bin/test.`uname -s`.`uname -m`.exe test.c
