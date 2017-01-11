{
    'targets': [
        {
            "variables": {
                "other_cflags%": "",
                "other_lflags%": "",
            },
            'target_name': 'boost_utils',
            'type': 'shared_library',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
                'endian/endian.hpp',
                'endian/endian.cpp',
                'fs/cached_file.hpp',
                'fs/cached_file.cpp',
                'netw/socket.hpp',
                'netw/socket.cpp',
                'log/log.cpp',
                'log/log.hpp',
                'log/log.h',
                'log/log.c',
                "boost-utils.hpp",
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'OTHER_CFLAGS': [
                            "<(other_cflags)",
                        ],
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lz",
                            "-Wl,-rpath,.",
                            "<(other_lflags)",
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'boost.utils.test',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
                'endian/endian_test.hpp',
                'fs/cached_file_test.hpp',
                'netw/socket_test.hpp',
                'log/log_test.hpp',
                'other_test.hpp',
                'all_test.cpp',
            ],
            'dependencies': [
                'boost_utils',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'OTHER_LDFLAGS': [
                            "-L.",
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lboost_unit_test_framework",
                            "-lboost_utils",
                            "-lz",
                            "-Wl,-rpath,.",
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'boost.utils.test.console',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
                'fs/cached_file_test_c.hpp',
                'netw/socket_test_c.hpp',
                'test/console_test.cpp',
            ],
            'dependencies': [
                'boost_utils',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'OTHER_LDFLAGS': [
                            "-L.",
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lboost_utils",
                            "-lz",
                            "-Wl,-rpath,.",
                        ],
                    },
                }],
            ],
        },
    ],
}
