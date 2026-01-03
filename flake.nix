{
  description = "AOC 2025 environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-stable";
  };

  outputs =
    { self, nixpkgs }:
    let
      # You can change this to "aarch64-linux" if you are on ARM
      system = "x86_64-linux";

      # Access the cross-compilation package set for 64-bit Windows
      pkgs = import nixpkgs {
        inherit system;
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [
          pkgs.buildPackages.gcc
          pkgs.buildPackages.binutils
          pkgs.clang_tools

        ];

        # Set environment variables to help your build system find the compiler
        shellHook = ''
          echo "AOC 2025 dev environment loaded"
          echo "Compiler: $CC"
        '';
      };
    };
}
