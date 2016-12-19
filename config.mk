PRJTYPE = Executable
LIBS = macu gfxwnd glfw glad
ifeq ($(OS), Windows_NT)
	LIBS += glu32 opengl32 gdi32 ole32 user32 shell32
else
	LIBS += GL X11 Xrandr Xinerama Xcursor pthread dl
endif
EXTDEPS = macu::0.0.2dev gfxwnd::0.0.0dev
