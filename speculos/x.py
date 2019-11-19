#!/usr/bin/env python3
import argparse
import os
import sys
import subprocess


def image_tag(target):
    return "ledger_dev_{}:latest".format(target)


def bolos_env(target, version):
    sdk = "nano{}-secure-sdk".format(target)

    if (target == "blue"):
        sdk = "blue-secure-sdk"

    return {
        "BOLOS_ENV": os.path.realpath('/opt/ledger/env'),
        "BOLOS_SDK": os.path.realpath('vendor/{}'.format(sdk)),
        "LEDGER_PROXY_ADDRESS": "127.0.0.1",
        "LEDGER_PROXY_PORT": "9999"
    }


def run_docker(target, version, command):
    image_name = image_tag(target)
    run_command = \
        "docker run --rm --privileged -v /dev:/dev \
            -v $(pwd)/..:/workspace {img} {t} {v} \"{cmd}\"\
         ".format(
            pwd=os.getcwd(),
            img=image_name,
            t=target,
            v=version,
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
            --network host \
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


def build_speculos():
    build_command = "mkdir -p build && \
            cmake -Bbuild -H. -DWITH_VNC=1 && \
            make -C build/".strip().split()
    build_command = " ".join(build_command)
    print(build_command)
    os.chdir("/workspace/vendor/speculos")
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
        "version",
        metavar="<sdk version>",
        type=str,
        nargs=1,
        help="SDK Version: 1.5.5 | 1.6.0"
    )
    parser.add_argument(
        "command",
        metavar='"<command>"',
        type=str,
        nargs=1,
        default="make",
        help="Command to Run in Speculos (quoted)"
    )

    args = parser.parse_args()
    target = args.target[0]
    version = args.version[0]
    command = args.command[0]

    if (sys.argv[0] != '/opt/ledger/x.py'):
        build_docker(target)
        run_docker(target, version, command)
        sys.exit(0)
    else:
        build_speculos()
        build_env = bolos_env(target, version)
        subprocess.run(
            "make",
            check=True,
            shell=True,
            env=build_env
        )
        speculos_command = "{speculos} --sdk {v} -d {app} & '{cmd}'".format(
            speculos="/workspace/vendor/speculos/speculos.py",
            v=version,
            app="/workspace/bin/app.elf",
            cmd=command
        )
        subprocess.run(
            speculos_command,
            check=True,
            shell=True,
            env=build_env
        )


if __name__ == "__main__":
    main()
