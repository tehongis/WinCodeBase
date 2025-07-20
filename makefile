CC=cl
CFLAGS=/nologo /W3 /EHsc /DUNICODE /D_UNICODE
LDFLAGS=/link user32.lib gdi32.lib

all: Z80me.exe

Z80me.exe: main.c 
    $(CC) $(CFLAGS) main.c /FeWinCodeBase.exe $(LDFLAGS)

clean:
    del WinCodeBase.exe main.obj
