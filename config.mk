PRJTYPE = Executable
LIBS = glfw glad
ifeq ($(OS), Windows_NT)
	LIBS += glu32 opengl32 gdi32 ole32 user32 shell32
endif
