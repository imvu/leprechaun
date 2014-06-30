import sys
import os

SRC = [
    'Application.cpp',
    'WindowClient.cpp'
]

env = Environment(
  ENV=os.environ,
)

OUTDIR = Dir('#/leprechaun-binaries')

CEFDIR = ARGUMENTS.get('CEFDIR', '/home/cit/src/chromium')
CEFCONFIG = ARGUMENTS.get('CEFCONFIG', 'Release')

env.Append(
    CEFDIR=CEFDIR,
    CEFCONFIG=CEFCONFIG,

    CPPPATH=[
        '$CEFDIR/src/cef'
    ],
)

if sys.platform.startswith('linux'):
    OUTDIR = OUTDIR.Dir('linux')

    SRC.extend(['linux_main.cpp',
                '$CEFBIN/libcef_dll_wrapper.a'])

    env.ParseConfig('pkg-config --cflags --libs gtk+-2.0')

    env.Append(
        CEFBIN = '$CEFDIR/src/out/$CEFCONFIG/obj.target/cef',
        LIBPATH=['$CEFDIR/src/out/$CEFCONFIG/lib.target/'],

    	CPPFLAGS=[
            '-O3',
            '-g'
        ],

        LIBS=[
            'cef',
            'crypto',
            'ssl'
        ]
    )
    
    program = env.Program(OUTDIR.File('leprechaun'), SRC)
    libcef = env.Install(OUTDIR, '$CEFBIN/libcef$SHLIBSUFFIX')
    locales = env.Install(OUTDIR, '$CEFDIR/src/out/$CEFCONFIG/locales')

    env.Default([
            program,
            libcef,
            locales
    ])

elif sys.platform == 'darwin':
    OUTDIR = OUTDIR.Dir('mac')

    SRC.append('mac_main.mm')
    SRC.append('$CEFBIN/libcef_dll_wrapper.a')

    env.Append(
        CCFLAGS=['-arch', 'i386', '-g', '-O0', '-fvisibility=hidden'],
        LINKFLAGS=['-arch', 'i386'],
        FRAMEWORKS=['CoreFoundation', 'AppKit'],
        CEFBIN='$CEFDIR/src/xcodebuild/$CEFCONFIG',
        LIBPATH=[
            '$CEFBIN'
        ],

        CPPFLAGS=[
            '-O3',
            '-g'
        ],
        LIBS=[
            'c++',
            'objc',
            'cef',
            'ssl'
        ]
    )

    BUNDLE = OUTDIR.Dir('leprechaun.app')

    libcef = env.File('$CEFBIN/libcef$SHLIBSUFFIX')

    process_helper = env.Program(
        'leprechaun Helper',
        ['process_helper.cpp',
         'Application.cpp',
         '$CEFBIN/libcef_dll_wrapper.a'])

    FRAMEWORKS = env.Dir('$CEFBIN/cefclient.app/Contents/Frameworks')

    d = env.Default([
        env.Install(BUNDLE.Dir('Contents'), 'Info.plist'),
        env.Install(BUNDLE.Dir('Contents/MacOS'), libcef),
        env.Program(BUNDLE.Dir('Contents/MacOS').File('leprechaun'), SRC),
        env.Install(BUNDLE.Dir('Contents/Frameworks'), FRAMEWORKS.Dir('Chromium Embedded Framework.framework/')),
        Command(BUNDLE.File('Contents/Frameworks/leprechaun Helper.app/Contents/Info.plist'), 'Helper.plist', Copy('$TARGET', '$SOURCE')),
        env.Install(BUNDLE.Dir('Contents/Frameworks/leprechaun Helper.app/Contents/MacOS'), [
            process_helper,
            libcef
            ])
    ])

else: # Windows
    OUTDIR = OUTDIR.Dir('windows')

    SRC.extend(['win_main.cpp'])

    env.Append(
        CEFBIN = '$CEFDIR/src/build/$CEFCONFIG',
        LIBPATH=[
	    '$CEFBIN/lib',
            'c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib',
            'C:\Program Files\Microsoft SDKs\Windows\\v6.0A\Lib',
        ],

	CPPPATH=[
            'C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include',
            'C:\Program Files\Microsoft SDKs\Windows\\v6.0A\Include',
        ],

	CPPFLAGS=[
            '/MT',
            '/EHsc',
            '/arch:SSE2',
        ],

	LINKFLAGS=[
            '/MACHINE:x86',
        ],

        LIBS=[
            'libcef_dll_wrapper',
            'libcef',
        ]
    )

    env.Replace(
        TARGET_ARCH='x86',
        CC='"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\cl.exe"',
        LINK='"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\link.exe"',
    )

    program = env.Program(OUTDIR.File('leprechaun.exe'), SRC)
    libcef = env.Install(OUTDIR, '$CEFBIN/libcef$SHLIBSUFFIX')
    locales = env.Install(OUTDIR, '$CEFDIR/src/build/$CEFCONFIG/locales')

    print env.Dump()

    env.Default([
            program,
            libcef,
            locales
    ])
