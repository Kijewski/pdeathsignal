#!/usr/bin/env python3

from setuptools import setup
from setuptools_rust import RustExtension
from setuptools.command.bdist_wheel import bdist_wheel


class bdist_wheel_abi3(bdist_wheel):
    def get_tag(self):
        python, abi, plat = super().get_tag()

        if python.startswith("cp"):
            # on CPython, our wheels are abi3 and compatible back to 3.10
            abi = "abi3"

        return python, abi, plat


if __name__ == "__main__":
    setup(
        rust_extensions=[
            RustExtension("pdeathsignal._pdeathsignal", "Cargo.toml", debug=False),
        ],
        cmdclass={
            "bdist_wheel": bdist_wheel_abi3,
        },
    )
