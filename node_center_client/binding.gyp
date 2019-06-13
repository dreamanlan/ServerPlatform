{
  "targets": [
    {
      "target_name": "center_client",
      "sources": [ "main.cc" ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      'conditions': [
        [ 'OS=="win"', {
          'libraries': [ '<(PRODUCT_DIR)/../../CenterClientLibrary.lib' ]
        }, { # POSIX
          'defines': [ '__POSIX__' ],
          'libraries': [ '-L/home/lanxiang/centerclient/lib', '-lCenterClientLibrary' ]
        }],
        ['OS=="mac"', {
          'cflags+': ['-fvisibility=hidden'],
          'xcode_settings': {
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
          }
        }],
      ],
    }
  ],
}
