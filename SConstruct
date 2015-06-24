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
    CEFRES='$CEFDIR/Resources',

    CPPPATH=[
        '$CEFDIR'
    ],
)

if sys.platform.startswith('linux'):
    OUTDIR = OUTDIR.Dir('linux')

    SRC.extend(['linux_main.cpp',
                '$CEFDIR/build/libcef_dll/libcef_dll_wrapper.a'])

    env.ParseConfig('pkg-config --cflags --libs gtk+-2.0')

    env.Append(
        LIBPATH=['$CEFDIR/$CEFCONFIG/'],

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
    libcef = env.Install(OUTDIR,'$CEFDIR/$CEFCONFIG/libcef$SHLIBSUFFIX') 
    libffmpeg = env.Install(OUTDIR,'$CEFDIR/$CEFCONFIG/libffmpegsumo$SHLIBSUFFIX') 
    natives_blob = env.Install(OUTDIR, '$CEFDIR/$CEFCONFIG/natives_blob.bin')
    snapshot_blob = env.Install(OUTDIR, '$CEFDIR/$CEFCONFIG/snapshot_blob.bin')
    locales = env.Install(OUTDIR, '$CEFRES/locales')
    cef_pak = env.Install(OUTDIR, '$CEFRES/cef.pak')
    devtools_pak = env.Install(OUTDIR, '$CEFRES/devtools_resources.pak')
    cef_100_pak = env.Install(OUTDIR, '$CEFRES/cef_100_percent.pak')
    cef_200_pak = env.Install(OUTDIR, '$CEFRES/cef_200_percent.pak')
    icudtl_dat = env.Install(OUTDIR, '$CEFRES/icudtl.dat')

    env.Default([
            program,
            libcef,
            libffmpeg,
            natives_blob,
            snapshot_blob,
            locales,
            cef_pak,
            devtools_pak,
            cef_100_pak,
            cef_200_pak,
            icudtl_dat
    ])

elif sys.platform == 'darwin':
    OUTDIR = OUTDIR.Dir('mac')

    SRC.append('mac_main.mm')
    SRC.append('$CEFDIR/build/libcef_dll/libcef_dll_wrapper.a')

    env.Append(
        CEFBIN = '$CEFDIR/$CEFCONFIG',
        CCFLAGS=['-arch', 'x86_64', '-g', '-O0', '-fvisibility=hidden'],
        LINKFLAGS=['-arch', 'x86_64'],
        FRAMEWORKS=['CoreFoundation', 'AppKit', 'Chromium Embedded Framework'],
        FRAMEWORKPATH=['$CEFBIN'],
        CEFBIN_FRAMEWORK='$CEFBIN/Chromium Embedded Framework.framework',

        CPPFLAGS=[
            '-O3',
            '-g'
        ],
        LIBS=[
            'stdc++.6',
            'objc',
            'ssl'
        ]
    )

    BUNDLE = OUTDIR.Dir('leprechaun.app')

    [leprechaun] = env.Program('leprechaun', SRC)
    def fixupLeprechaun(target, source, env):
        [target] = target
        env.Execute('install_name_tool -change "@executable_path/Chromium Embedded Framework" "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" {0}'.format(target.path))
    env.AddPostAction(leprechaun, fixupLeprechaun)

    [process_helper] = env.Program(
        'leprechaun Helper',
        ['process_helper.cpp',
         'Application.cpp',
         'WindowClient.cpp',
         '$CEFDIR/build/libcef_dll/libcef_dll_wrapper.a'])
    def fixupHelper(target, source, env):
        [target] = target
        env.Execute('install_name_tool -change "@executable_path/Chromium Embedded Framework" "@executable_path/../../../../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" "{0}"'.format(target.path))
    env.AddPostAction(process_helper, fixupHelper)

    d = env.Default([
        env.Install(BUNDLE.Dir('Contents'), 'Info.plist'),
        env.Install(BUNDLE.Dir('Contents/MacOS'), [leprechaun]),
        env.Install(BUNDLE.Dir('Contents/Frameworks'), '$CEFBIN_FRAMEWORK'),
        Command(BUNDLE.File('Contents/Frameworks/leprechaun Helper.app/Contents/Info.plist'), 'Helper.plist', Copy('$TARGET', '$SOURCE')),
        env.Install(BUNDLE.Dir('Contents/Frameworks/leprechaun Helper.app/Contents/MacOS'), [
            [process_helper]
            ]),
    ])

else: # Windows
    OUTDIR = OUTDIR.Dir('windows')

    SRC.extend(['win_main.cpp'])

    def getEnvPath(var, cefPaths):
        rawLibPath = os.environ[var]
        envLibPath = filter(lambda x: len(x) > 0, rawLibPath.split(';'))
        return envLibPath + cefPaths

    env.Append(
        CEFBIN = '$CEFDIR\\$CEFCONFIG',

        LIBPATH=getEnvPath('LIBPATH', [
            os.environ['WindowsSdkDir'] + '\\Lib',
            '$CEFBIN',
            '$CEFDIR\\build\\libcef_dll'
        ]),

        CPPPATH=getEnvPath('INCLUDE', [
            '$CEFDIR\\include'
        ]),

        CPPFLAGS=[
            '/MT',
            '/O2',
            '/Ob2',
            '/GF',
            '/D NDEBUG',
            '/D _NDEBUG',
        ],

        LINKFLAGS=[
            '/MACHINE:x86',
        ],

        LIBS=[
            'libcef_dll_wrapper',
            'libcef',
            'comctl32',
            'rpcrt4',
            'shlwapi',
            'kernel32'
        ]
    )

    env.Replace(
        TARGET_ARCH='x86',
        CC='"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\cl.exe"',
        LINK='"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\link.exe"',
    )

    env.Default([
        env.Program(OUTDIR.File('leprechaun.exe'), SRC),
        env.Install(OUTDIR, '$CEFBIN\\libcef$SHLIBSUFFIX'),
        env.Install(OUTDIR, '$CEFRES\\locales'),
        env.Install(OUTDIR, '$CEFRES\\cef.pak'),
        env.Install(OUTDIR, '$CEFRES\\cef_100_percent.pak'),
        env.Install(OUTDIR, '$CEFRES\\cef_200_percent.pak'),
        env.Install(OUTDIR, '$CEFRES\\devtools_resources.pak'),
        env.Install(OUTDIR, '$CEFRES\\icudtl.dat'),
    ])
