{
    'targets': [
        {
            'target_name': 'boost-utils',
            'type': 'shared_library',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
                'endian/endian.hpp',
                'endian/endian.cpp',
                'netw/socket.hpp',
                'netw/socket.cpp',
                "boost-utils.hpp",
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.1',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lcwf",
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
                'netw/socket_test.hpp',
                'log/log_test.hpp',
                'other_test.hpp',
                'all_test.cpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.1',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lcwf",
                            "-lboost_unit_test_framework",
                            "-lboost-utils",
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
                'test/console_test.cpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.1',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_thread",
                            "-lcwf",
                            "-lboost-utils",
                        ],
                    },
                }],
            ],
        },
    ],
}
