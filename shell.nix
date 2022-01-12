with import <nixpkgs> {};

mkShell {
  buildInputs = [ libglvnd xorg.libXinerama xorg.libXext xorg.libX11 xorg.libXtst xorg.libXi ];
}
