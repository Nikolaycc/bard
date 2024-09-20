{
  description = "Bard Programming Language Development Environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";  # Consider using a stable tag for reproducibility
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let
    pkgs = import nixpkgs {
      inherit system;
      overlays = [];  # Add your overlays here if needed
    };
  in {
    devShells.default = pkgs.mkShell rec {
      name = "bard-lang-dev";

      buildInputs = with pkgs; [
        llvm_16
        llvmPackages_16.clang
        llvmPackages_16.lld
        pkg-config
        gnumake
        libffi
        libedit
        ncurses
        zlib
      ];

      shellHook = ''
        export CC=clang
        export CXX=clang++
        export LLVM_CONFIG=$(which llvm-config)
        export LD_LIBRARY_PATH=${pkgs.llvm_16}/lib:$LD_LIBRARY_PATH
        echo "Development environment with LLVM 16 and Clang 16 (Flake)"
      '';
    };
  });
}
