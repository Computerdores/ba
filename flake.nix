{
    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/release-25.05";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, flake-utils}: flake-utils.lib.eachDefaultSystem(system:
        let
            pkgs = import nixpkgs { inherit system; };

            tex = pkgs.texlive.combine {
                inherit (pkgs.texlive)
                    scheme-basic

                    todonotes

                    lineno
                    upquote
                    minted

                    siunitx

                    caption

                    cleveref

                    latexmk
                    biber
                    biblatex
                    luatex;
            };
        in {
            devShells.default = pkgs.mkShell {
                hardeningDisable = [ "all" ];
                nativeBuildInputs = with pkgs; [
                    cmake
                    automake
                    gcc14
                    pkg-config
                    cxxopts
                ];

                buildInputs = (with pkgs; [
                    python3
                    unzip

                    tex
                    tex-fmt
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
                    PS1="''${PS1/\\n/\\n(devenv) }"
                    alias clion="nohup clion . >/dev/null 2>/dev/null &"
                '';
            };
        }
    );
}
