#!/usr/bin/env bash
echo "Start update code routine!"
fetch=$(sudo git fetch)

if [[ -z "$fetch" ]] 
then 
    echo "Local Repo up to date, no git pull needed" && exit 1 
else 
    sudo git pull || echo "git pull failed, exiting." ; exit 1 
fi
echo "Make source code"
make  