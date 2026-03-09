#!/usr/bin/env zsh

ffmpeg -framerate 30 -i frames/frame-%02d.ppm -c:v libx264 -pix_fmt yuv420p frames/output.mp4
open frames/output.mp4

