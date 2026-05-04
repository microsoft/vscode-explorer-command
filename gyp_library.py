#!/usr/bin/env python3
import os
import sys

root = os.path.dirname(__file__)
sys.path.insert(0, os.path.join(root, 'node-gyp', 'gyp', 'pylib'))
import gyp

def run_gyp(arch, extra_args):
    args = list(extra_args)
    args.extend([
        'main.gyp',
        '--depth=.',
        '--generator-output=out',
        '-Dlibrary=shared_library',
        f'-Dtarget_arch={arch}',
    ])

    rc = gyp.main(args)
    if rc:
        sys.exit(rc)


if __name__ == '__main__':
  run_gyp(sys.argv[1], sys.argv[2:])