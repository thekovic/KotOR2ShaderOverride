#!/bin/bash

find ./shaders_override -type f -exec sed -i -f edit_shaders.sed {} \;

