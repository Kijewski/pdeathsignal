#!/usr/bin/env python

from setuptools import setup, Extension


setup(
    name='pdeathsignal',
    version='0.0.1',
    description='Get and set the parent process death signal.',
    ext_modules=[Extension(
        'pdeathsignal',
        sources=['pdeathsignal.c'],
        extra_compile_args=[
            '-std=c99',
            '-fPIC', '-pipe',
            '-D_FORTIFY_SOURCE=2', '-fstack-protector-strong', '--param=ssp-buffer-size=8',
            '-O2', '-ggdb1', '-fomit-frame-pointer',
        ],
        extra_link_args=[
            '-fPIC', '-pipe',
            '-Wl,-zrelro,-znow,-zcombreloc,-znocommon,-znoexecstack,-znodump',
        ],
    )],
)
