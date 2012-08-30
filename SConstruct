
SRC = [
    'main.cpp',
    '$CEFBIN/obj.target/cef/libcef_dll_wrapper.a',
]

env = Environment()

env.Append(
    CEF='/home/cit/src/chromium',
    CEFBIN='$CEF/src/out/Release/',

    CPPPATH=[
        '$CEF/src/cef'
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

program = env.Program('leprechaun', SRC)
libcef = env.Install('#', '$CEFBIN/obj.target/cef/libcef.so')

env.Default([program, libcef])
