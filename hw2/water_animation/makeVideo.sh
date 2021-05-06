#!/bin/bash

videoName=$1

cmd="ffmpeg -r 30 -i ${videoName}_%04d.png -acodec none -vcodec libx264 -pix_fmt yuv420p $videoName.mp4"
echo $cmd
$cmd
