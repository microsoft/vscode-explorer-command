#!/usr/bin/env python3
from __future__ import print_function
import ast
import os
import pprint
import sys

root = os.path.dirname(__file__)

sys.path.insert(0, os.path.join(root, 'node-gyp', 'gyp', 'pylib'))
import gyp

def edit_config_gypi(arch):
  config_gypi = os.path.join(root, 'config.gypi')
  with open(config_gypi, 'r') as f:
    content = f.read()
    config = ast.literal_eval(content)
    v = config['variables']
    v['target_arch'] = arch
    with open(config_gypi, 'w+') as f:
      f.write(pprint.pformat(config, indent=2))

def edit_main_gyp(arch):
  main_gyp = os.path.join(root, 'main.gyp')
  with open(main_gyp, 'r') as f:
    content = f.read()
    config = ast.literal_eval(content)
    v = config['target_defaults']
    v['msvs_configuration_platform'] = 'ARM64'
    with open(main_gyp, 'w+') as f:
      f.write(pprint.pformat(config, indent=2))

def run_gyp(arch, args):
  edit_config_gypi(arch)
  if arch == 'arm64':
    edit_main_gyp(arch)

  args.append('main.gyp')
  args.extend(['-I', 'config.gypi'])
  args.append('--depth=.')
  args.append('--generator-output=out')
  args.append('-Dlibrary=shared_library')

  rc = gyp.main(args)
  if rc != 0:
    print('Error running GYP')
    sys.exit(rc)


if __name__ == '__main__':
  run_gyp(sys.argv[1], sys.argv[2:])