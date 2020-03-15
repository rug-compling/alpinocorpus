{
  pkgs ? import <nixpkgs> {}
}:

with pkgs;

stdenv.mkDerivation {
  name = "alpinocorpus";

  src = nix-gitignore.gitignoreSource [ ".git/" ] ./.;

  nativeBuildInputs = [
    cmake
    pkgconfig
  ];

  buildInputs = [
    boost
    dbxml
    libxml2
    libxslt
    xercesc
    xqilla
    zlib
  ] ++ lib.optional stdenv.isDarwin libiconv;

  doInstallCheck = true;

  # Tests currently only work after installation, since the library
  # paths are not set up correctly.
  installCheckPhase = ''
    make test
  '';

   meta = with stdenv.lib; {
    description = "Library for Alpino treebanks";
    homepage = "https://github.com/rug-compling/alpinocorpus";
    license = licenses.lgpl21;
    platforms = platforms.unix;
  }; 
}
