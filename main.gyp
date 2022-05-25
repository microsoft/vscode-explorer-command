{
  'target_defaults': {
    'conditions': [
      [ 'OS=="win"', {
        'sources': [
          'src/explorer_command.cc',
          'src/explorer_command.def',
        ],
        'defines': [
          '_WINDLL',
          'WIN32_LEAN_AND_MEAN',
          '_UNICODE',
          'UNICODE',
          '_CRT_SECURE_NO_DEPRECATE',
          '_CRT_NONSTDC_NO_DEPRECATE',
          '_HAS_EXCEPTIONS=0',
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalOptions': [
              '/guard:cf',
            ],
            'OptimizeReferences': 2,             # /OPT:REF
            'EnableCOMDATFolding': 2,            # /OPT:ICF
          },
          'VCCLCompilerTool': {
            'AdditionalOptions': [
              '/Zc:__cplusplus',
              '-std:c++17',
              '/Qspectre',
              '/guard:cf',
            ],
            'BufferSecurityCheck': 'true',
            'ExceptionHandling': 0,               # /EHsc
            'EnableFunctionLevelLinking': 'true',
            'Optimization': 3,              # /Ox, full optimization
          },
        },
        'libraries': [
          '-ladvapi32.lib',
          '-lruntimeobject.lib',
          '-lshlwapi.lib',
        ]
      }],
    ],
  },
  'targets': [{
    'target_name': 'code_explorer_command',
    'type': 'shared_library',
    'defines': [
      'REGISTRY_LOCATION="*\\\\shell\\\\VSCode"',
    ],
    'conditions': [
      [ 'OS=="win"', {
        'conditions': [
          ['target_arch=="x86"', {
            'TargetMachine' : 1,              # /MACHINE:X86
            'defines': [ 'DLL_UUID="0632BBFB-D195-4972-B458-53ADEB984588"', ],
          }],
          ['target_arch=="x64"', {
            'TargetMachine' : 17,             # /MACHINE:X64
            'defines': [ 'DLL_UUID="1C6DF0C0-192A-4451-BE36-6A59A86A692E"', ],
          }],
          ['target_arch=="arm64"', {
            'TargetMachine' : 0,
            'defines': [ 'DLL_UUID="F5EA5883-1DA8-4A05-864A-D5DE2D2B2854"', ],
          }],
        ],
      }],
    ],
  }, {
    'target_name': 'code_insiders_explorer_command',
    'type': 'shared_library',
    'defines': [
      'REGISTRY_LOCATION="*\\\\shell\\\\VSCodeInsiders"',
    ],
    'conditions': [
      [ 'OS=="win"', {
        'conditions': [
          ['target_arch=="x86"', {
            'TargetMachine' : 1,              # /MACHINE:X86
            'defines': [ 'DLL_UUID="B9949795-B37D-457F-ADDE-6A950EF85CA7"', ],
          }],
          ['target_arch=="x64"', {
            'TargetMachine' : 17,             # /MACHINE:X64
            'defines': [ 'DLL_UUID="799F4F7E-5934-4001-A74C-E207F44F05B8"', ],
          }],
          ['target_arch=="arm64"', {
            'TargetMachine' : 0,
            'defines': [ 'DLL_UUID="7D34756D-32DD-4EE6-B99F-2691C0DAD875"', ],
          }],
        ],
      }],
    ],
  }],
}