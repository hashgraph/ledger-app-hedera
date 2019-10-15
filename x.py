#!/usr/bin/env python3

# This script is intended to simplify usage inside of
# the docker development environment for Ledger

# usage: ./x.py [target] ...

import sys
import subprocess
import os
from os import path

# Useless if there aren't at least 2 arguments
if len(sys.argv) < 3:
    print("usage: x.py <target> <args>")
    sys.exit(1)

# The #1 argument is always the <target>
target = sys.argv[1]

# Validate that this is an allowed target
TARGETS = ['x', 's', 'blue']
if target not in TARGETS:
    print(f"target '{target}' not one of 's', 'x', or 'blue'")
    sys.exit(0)

# Check if we are inside the Docker environment
if sys.argv[0] != '/opt/x.py':
    # Build the docker envrionment image
    out = subprocess.run(
        'docker build -q .',
        check=True, shell=True,
        capture_output=True)

    image = out.stdout.decode().strip()

    # Re-run the script from within the docker environment
    pwd = os.getcwd()
    cmd = ' '.join(sys.argv[1:])
    subprocess.run(
        f'docker run --rm --privileged -v {pwd}:/workspace {image} {cmd}',
        shell=True, check=True)

    sys.exit(0)

# Setup BOLOS_SDK and BOLOS_ENV for the target
if target == 's':
    bolos_sdk = path.realpath('vendor/nanos-secure-sdk')
    bolos_env = path.realpath('/opt/ledger/others')
elif target == 'x':
    bolos_sdk = path.realpath('vendor/nanox-secure-sdk')
    bolos_env = path.realpath('/opt/ledger/nanox')
elif target == 'blue':
    bolos_sdk = path.realpath('vendor/blue-secure-sdk')
    bolos_env = path.realpath('/opt/ledger/others')

if sys.argv[2] == 'make':
    # Now, we run the rest of the arg list as a subcommand as if
    # it was typed in the shell
    subprocess.run(sys.argv[2:], check=True, env={
        'BOLOS_SDK': bolos_sdk, 'BOLOS_ENV': bolos_env})

elif sys.argv[2] == 'proto':
    # Generate C files using protoc from protobufs
    plugin = path.realpath('vendor/ledger-nanopb/generator/protoc-gen-nanopb')
    plugin = f'--plugin=protoc-gen-nanopb={plugin}'
    proto = path.realpath('proto')
    nanopb = path.realpath('vendor/ledger-nanopb/generator/proto')

    subprocess.run('make', shell=True, check=True, cwd=nanopb)
    subprocess.run(
        f'protoc {plugin} --nanopb_out=. -I. -I{nanopb} *.proto',
        shell=True, check=True,
        cwd=proto)
