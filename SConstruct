import sys

SRC = [
    'Application.cpp',
    'WindowClient.cpp'
]

env = Environment()

OUTDIR = Dir('#/leprechaun-binaries')

CEFDIR = ARGUMENTS.get('CEFDIR', '/home/cit/src/chromium')
CEFCONFIG = ARGUMENTS.get('CEFCONFIG', 'Release')

env.Append(
    CEFDIR=CEFDIR,
    CEFCONFIG=CEFCONFIG,

    CPPPATH=[
        '$CEFDIR/src/cef'
    ],

    CPPFLAGS=[
        '-O3',
        '-g'
    ],

    LIBS=[
        'ssl'
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

        LIBS=[
            'cef'
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
        LIBS=[
            'c++',
            'objc',
            'cef'
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
