
SRC = [
    'main.cpp',
    '$CEFBIN/obj.target/cef/libcef_dll_wrapper.a',
]

env = Environment()

OUTDIR = Dir('#/build')

env.Append(
    CEF='/home/cit/src/chromium',
    CEFBIN='$CEF/src/out/Release/',
    #CEFBIN='$CEF/src/out/Debug/',

    CPPPATH=[
        '$CEF/src/cef'
    ],

    CPPFLAGS=[
        '-O3',
        '-g'
    ],

    LIBPATH=[
        '$CEFBIN/obj.target/cef'
    ],

    LIBS=[
        'cef',
        'ssl'
    ],
)

env.ParseConfig('pkg-config --cflags --libs gtk+-2.0')

program = env.Program(OUTDIR.File('leprechaun'), SRC)
libcef = env.Install(OUTDIR, '$CEFBIN/obj.target/cef/libcef.so')
locales = env.Install(OUTDIR, '$CEF/src/out/Release/locales')

env.Default([program, libcef, locales])
