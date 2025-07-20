# Compiler and flags
CC = cl
CFLAGS = /nologo /EHsc /Zi /MD
LIBS = d3d11.lib d3dcompiler.lib dxguid.lib user32.lib gdi32.lib

# Source and output
SRC = main.c
OUT = WinCodeBase.exe

all: $(OUT)

$(OUT): $(SRC)
    $(CC) $(SRC) /Fe$(OUT) $(CFLAGS) $(LIBS)

clean:
    del *.obj *.exe *.pdb
