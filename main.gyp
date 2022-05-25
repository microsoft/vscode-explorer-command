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
      'DLL_UUID="C490C2CA-5E94-4137-A35E-0611F3C93EC5"',
    ]
  }, {
    'target_name': 'code_insiders_explorer_command',
    'type': 'shared_library',
    'defines': [
      'REGISTRY_LOCATION="*\\\\shell\\\\VSCodeInsiders"',
      'DLL_UUID="FDD3F547-CFE9-4526-9FFF-CF8A4DF7DB4C"',
    ]
  }],
}