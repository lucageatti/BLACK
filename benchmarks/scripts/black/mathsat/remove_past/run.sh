#!/bin/bash

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$HOME/.local/lib:$HOME/.local/lib64"
black solve --remove-past "$1"
