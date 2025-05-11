{
    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, flake-utils}: flake-utils.lib.eachDefaultSystem(system:
        let
            pkgs = import nixpkgs { inherit system; };
            nativeBuildDeps = with pkgs; [
                cmake
                automake
                gcc14
                pkg-config
            ];
            buildDeps = with pkgs; [];
        in {
            devShells.default = pkgs.mkShell {
                hardeningDisable = [ "all" ];
                nativeBuildInputs = nativeBuildDeps;

                buildInputs = buildDeps;
            };
        }
    );
}
