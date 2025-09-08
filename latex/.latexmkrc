$pdf_mode = 4; # use lualatex
$postscript_mode = $dvi_mode = 0; # recommended setting for lualatex according to https://ftp.rrze.uni-erlangen.de/ctan/support/latexmk/example_rcfiles/lualatex_latexmkrc

add_cus_dep('glo', 'gls', 0, 'run_makeglossaries');
add_cus_dep('acn', 'acr', 0, 'run_makeglossaries');
set_tex_cmds( '--shell-escape --halt-on-error %O %S' );

# simplified to version from https://tex.stackexchange.com/questions/123183/how-do-i-set-up-my-config-file-to-have-latexmk-default-to-making-a-pdf
sub run_makeglossaries {
    if ( $silent ) {
        system "makeglossaries -q '$_[0]'";
    }
    else {
        system "makeglossaries '$_[0]'";
    };
}

push @generated_exts, 'glo', 'gls', 'glg';
push @generated_exts, 'acn', 'acr', 'alg';
push @generated_exts, 'tdo', 'sta';
$clean_ext .= ' %R.ist %R.xdy';
