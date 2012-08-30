
SRC = [
    'main.cpp'
]

env = Environment()

env.Append(
    CEF='/home/cit/src/chromium',
    CEFBIN='$CEF/src/out/Release/',

    CPPPATH=[
        '$CEF/src/cef'
    ],
    LIBPATH=[
        '$CEFBIN/lib.target'
    ],

    LIBS=[
        'cef',
        'ssl'
    ],
)

env.ParseConfig('pkg-config --cflags --libs gtk+-2.0')

program = env.Program('leprechaun', SRC)
libcef = env.Install('#', '$CEFBIN/lib.target/libcef.so')

env.Default([program, libcef])
