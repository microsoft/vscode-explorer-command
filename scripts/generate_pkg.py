#!/usr/bin/env python3
from __future__ import print_function
from shutil import copy
import os
import sys

code_clsid_map = {
  'x86': '0632BBFB-D195-4972-B458-53ADEB984588',
  'x64': '1C6DF0C0-192A-4451-BE36-6A59A86A692E',
  'arm64': 'F5EA5883-1DA8-4A05-864A-D5DE2D2B2854'
}

code_insiders_clsid_map = {
  'x86': 'B9949795-B37D-457F-ADDE-6A950EF85CA7',
  'x64': '799F4F7E-5934-4001-A74C-E207F44F05B8',
  'arm64': '7D34756D-32DD-4EE6-B99F-2691C0DAD875'
}

root = os.path.dirname(os.path.dirname(__file__))
out_dir = os.path.join(root, 'out')
pkg_type = sys.argv[1]
arch = sys.argv[2]
pkg_dir = os.path.join(out_dir, pkg_type + '_explorer_pkg_' + arch)

# Copy generated dll file to the package directory.
os.mkdir(pkg_dir)
dll = os.path.join(out_dir, 'Default', pkg_type + '_explorer_command.dll')
copy(dll, pkg_dir)

# Update AppxManifest.
manifest = os.path.join(root, 'template', 'AppxManifest.xml')
with open(manifest, 'r') as f:
  content = f.read()
  content = content.replace('@@PackageArch@@', arch)
  content = content.replace('@@PackageDLL@@', pkg_type + '_explorer_command.dll')
  content = content.replace('@@PackageDescription@@', pkg_type + ' context menu handler')
  content = content.replace('@@PackageLogo@@', pkg_type)
  if pkg_type == 'code':
    content = content.replace('@@PackageName@@', 'code')
    content = content.replace('@@PackageDisplayName@@', 'Visual Studio Code')
    content = content.replace('@@ApplicationId@@', 'Code')
    content = content.replace('@@ApplicationIdShort@@', 'Code')
    content = content.replace('@@MenuID@@', 'OpenWithCode')
    content = content.replace('@@CLSID@@', code_clsid_map[arch])
  if pkg_type == 'code_insiders':
    content = content.replace('@@PackageName@@', 'code-insiders')
    content = content.replace('@@PackageDisplayName@@', 'Visual Studio Code - Insiders')
    content = content.replace('@@ApplicationId@@', 'Code - Insiders')
    content = content.replace('@@ApplicationIdShort@@', 'CodeInsiders')
    content = content.replace('@@MenuID@@', 'OpenWithCodeInsiders')
    content = content.replace('@@CLSID@@', code_insiders_clsid_map[arch])

# Copy AppxManifest file to the package directory.
manifest_output = os.path.join(pkg_dir, 'AppxManifest.xml')
with open(manifest_output, 'w+') as f:
  f.write(content)

# Copy Assets to the package directory.
assets = os.path.join(root, 'assets')
copy(os.path.join(assets, pkg_type + '_150x150.png'), pkg_dir)
copy(os.path.join(assets, pkg_type + '_70x70.png'), pkg_dir)