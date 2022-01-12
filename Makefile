all : fmouse
.PHONY: run clean

#for X11 consider:             xorg-dev
#for X11, you will need:       libx-dev
#for full screen you'll need:  libxinerama-dev libxext-dev
#for OGL You'll need:          mesa-common-dev libglu1-mesa-dev

#-DCNFGRASTERIZER
#  and
#-CNFGOGL
#  are incompatible.


fmouse : fmouse.c rawdraw_sf.h monospace_pixels.h permutations.h
	gcc -o $@ $^ -lX11 -lpthread -lXinerama -lXtst -lXext -DHAS_XINERAMA -DHAS_XSHAPE -lm -ldl

run: fmouse.c
	./fmouse

clean :
	rm -rf *.o *~ rawdraw.exe rawdraw ontop rawdraw_mac rawdraw_mac_soft rawdraw_mac_cg rawdraw_mac_ogl ogltest ogltest.exe rawdraw_egl fmouse fmouse_ogl fmouse.exe
