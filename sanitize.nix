{ pkgs ? import <nixpkgs> {} }:

(import ./default.nix { inherit pkgs; }).overrideAttrs (attr: {
  NIX_CFLAGS_COMPILE = "-fsanitize=address -fsanitize=undefined";
})
