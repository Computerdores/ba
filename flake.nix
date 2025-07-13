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

                buildInputs = buildDeps ++ (with pkgs; [
                    python3
                    nbstripout
                ]) ++ (with pkgs.python3Packages; [
                    pandas
                    matplotlib
                    # ipynb
                    ipykernel
                    jupyter
                    jupyterlab
                    ipympl
                    jupyterlab-widgets
                    jupytext
                    seaborn
                ]);

                shellHook = ''
                    nbstripout --install
                    PS1="''${PS1/\\n/\\n(devenv) }"
                    alias clion="nohup clion . >/dev/null 2>/dev/null &"
                '';
            };
        }
    );
}
