#!/usr/bin/env python3
from __future__ import print_function
import os
import sys

root = os.path.dirname(__file__)

sys.path.insert(0, os.path.join(root, 'node-gyp', 'gyp', 'pylib'))
import gyp

# Directory within which we want all generated files
# to be written.
output_dir = os.path.join(os.path.abspath(root), 'out')

def run_gyp(args):
  args.append(os.path.join(root, 'main.gyp'))
  args += ['-f', 'msvs']
  args.append('--depth=' + root)
  args += ['--generator-output', output_dir]
  args.append('-Dlibrary=shared_library')

  rc = gyp.main(args)
  if rc != 0:
    print('Error running GYP')
    sys.exit(rc)


if __name__ == '__main__':
  run_gyp(sys.argv[1:])