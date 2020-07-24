{
  pkgs ? import <nixpkgs> {}
}:

with pkgs;

stdenv.mkDerivation {
  name = "alpinocorpus";

  src = nix-gitignore.gitignoreSource [ ".git/" ] ./.;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
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

  doCheck = true;

  # Meson is no longer able to pick up Boost automatically.
  # https://github.com/NixOS/nixpkgs/issues/86131
  BOOST_INCLUDEDIR = "${stdenv.lib.getDev boost}/include";
  BOOST_LIBRARYDIR = "${stdenv.lib.getLib boost}/lib";

  meta = with stdenv.lib; {
    description = "Library for Alpino treebanks";
    homepage = "https://github.com/rug-compling/alpinocorpus";
    license = licenses.lgpl21;
    platforms = platforms.unix;
  }; 
}
