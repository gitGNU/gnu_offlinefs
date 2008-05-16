#!/bin/bash
export XAUTHORITY=/root/.Xauthority
export DISPLAY=:0.0
xauth merge /home/curro/.Xauthority
exec kdialog --warningcontinuecancel  "Insert volume: \"$1\"" --title "Insert volume" --name offlinefs --caption offlinefs
