{
  description = "Tools for developing and building the dc32-badge";
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
                picotool = pkgs.callPackage ./picotool.nix {
                  pico-sdk = pkgs.callPackage ./pico-sdk.nix { };
                };
                pico-sdk = pkgs.callPackage ./pico-sdk.nix { };
              })
            ];
          };
        in
        {
          # Developmnt shell (nix develop)
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
          # Build
          packages.dc32-badge = pkgs.callPackage ./default.nix { };
        }
      );
}
