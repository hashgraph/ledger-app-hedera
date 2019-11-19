#!/usr/bin/env python3
import argparse
import os
import sys
import subprocess


def image_tag(target):
    return "ledger_dev_{}:latest".format(target)


def bolos_env(target):
    sdk = "nano{}-secure-sdk".format(target)

    if (target == "blue"):
        sdk = "blue-secure-sdk"

    return {
        "BOLOS_ENV": os.path.realpath('/opt/ledger/env'),
        "BOLOS_SDK": os.path.realpath('vendor/{}'.format(sdk))
    }


def run_docker(target, command):
    image_name = image_tag(target)
    run_command = \
        "docker run --rm --privileged -v /dev:/dev \
            -v $(pwd)/..:/workspace {img} {t} \"{cmd}\"\
         ".format(
            pwd=os.getcwd(),
            img=image_name,
            t=target,
            cmd=command
        ).strip().split()
    run_command = " ".join(run_command)
    print(run_command)
    subprocess.run(
        run_command,
        shell=True,
        check=True
    )


def build_docker(target):
    image_name = image_tag(target)
    user_id = os.geteuid()
    group_id = os.getgid()
    clang_version = "4.0.0"

    if (target == "x"):
        clang_version = "7.0.1"

    build_command = \
        "docker image build -q \
            -t {name} \
            --build-arg USER_ID={user} \
            --build-arg GROUP_ID={group} \
            --build-arg CLANG_VERSION={clang} .\
        ".format(
            name=image_name,
            user=user_id,
            group=group_id,
            clang=clang_version
        ).strip().split()
    build_command = " ".join(build_command)
    print(build_command)
    subprocess.run(
        build_command,
        shell=True,
        check=True
    )


def main():
    parser = argparse.ArgumentParser(
        description="Run Ledger Development Commands"
    )
    parser.add_argument(
        "target",
        metavar="<target>",
        type=str,
        nargs=1,
        help="Target Device (s, x, blue)"
    )
    parser.add_argument(
        "command",
        metavar='"<command>"',
        type=str,
        nargs=1,
        default="make",
        help="Command to Run (quoted)"
    )

    args = parser.parse_args()
    target = args.target[0]
    command = args.command[0]

    if (sys.argv[0] != '/opt/ledger/x.py'):
        build_docker(target)
        run_docker(target, command)
        sys.exit(0)
    else:
        subprocess.run(
            command,
            check=True,
            shell=True,
            env=bolos_env(target)
        )


if __name__ == "__main__":
    main()
