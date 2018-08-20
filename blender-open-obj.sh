#!/usr/bin/env bash

if [ -z "$1" ] ; then
	echo "usage: $0 <mesh.obj>"
	exit 1
fi

export OPEN_OBJ="$1"
blender -P blender-open-obj.py
