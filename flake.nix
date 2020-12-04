{
  description = "Alpino corpus utilities";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-20.09";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
  utils.lib.eachSystem  [ "x86_64-linux" ] (system: {
    defaultPackage = self.packages.${system}.alpinocorpus;

    packages.alpinocorpus = with nixpkgs.legacyPackages.${system}; stdenv.mkDerivation {
      pname = "alpinocorpus";
      version = "3.0.0";

      src = ./.;

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

      outputs = [ "bin" "dev" "out" ];

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
    };
  });
}
