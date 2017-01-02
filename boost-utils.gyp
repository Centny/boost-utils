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
                            "-lboost_filesystem",
                            "-lboost_thread",
                            "-Wall",
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'boost-utils-test',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
                'endian/endian.hpp',
                'endian/endian.cpp',
                "boost-utils.hpp",
                'endian/endian_test.hpp',
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
                            "-lboost_unit_test_framework",
                        ],
                    },
                }],
            ],
        },
    ],
}
