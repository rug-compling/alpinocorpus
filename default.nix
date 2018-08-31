with import <nixpkgs> {};
stdenv.mkDerivation rec {
  name = "alpinocorpus-env";
  env = buildEnv { name = name; paths = buildInputs; };

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    boost
    dbxml
    libxml2
    libxslt
    pkgconfig
    xercesc
    xqilla
    zlib
  ];
}
