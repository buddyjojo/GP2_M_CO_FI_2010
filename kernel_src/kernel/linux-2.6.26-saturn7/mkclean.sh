#!/bin/sh

image_type="normal flash"

make mrproper

for type in ${image_type}; do
	cp config-${type} obj-${type}/.config
	make O=obj-${type} clean
	echo -ne "\n\n"
done
