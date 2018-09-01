with import <nixpkgs> {};
stdenv.mkDerivation rec {
  name = "alpinocorpus-env";
  env = buildEnv { name = name; paths = buildInputs; };

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
  ];
}
