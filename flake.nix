{
  description = "Tools for developing, building, flashing, and debugging the slab keyboard";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
    flake-utils.url = "github:numtide/flake-utils";
    xc.url = "github:joerdav/xc";
  };
  outputs = { self, nixpkgs, flake-utils, ... }@inputs:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [
              (final: prev: {
                xc = inputs.xc.packages.x86_64-linux.xc;
              })
            ];
          };
        in
        {
          devShells.default = pkgs.mkShell {
            buildInputs = with pkgs; [
              xc

              cmake
              gcc
              ccls
              gcc-arm-embedded
              openocd
              picotool
              python39
              minicom
              gnumake
              gdb
              git
              cacert
            ];
          };
        }
      );
}
