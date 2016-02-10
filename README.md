[![Build Status](https://travis-ci.org/acwtools/accss.svg?branch=master)](https://travis-ci.org/acwtools/accss)

# ACCSS - a css compressor written in ansi c

## Install:

### Unix/OSX
- clone git repository
  ```
      git clone git://github.com/acwtools/accss
  ```
- OR download latest archive  
- untar / unzip archive
- change directory to unpacked files
- Prepare with:
    ```
        ./configure
    ```
- Build src
    ```
        make
    ```
- Install
    ```
        sudo make install
    ```
    For detailed configure options, see INSTALL file

### Windows
- download latest archive from download section
- unzip archive
- copy accss.exe in a directory in %PATH% Variable

## Usage:
```
    accss
        reads from stdin, prints to stdout
    accss <filename> [<filename>...]
        minimizes the CSS in files <filename> and outputs the result to stdout
        multiple files while be combined to one output stream
    accss -o <out_filename>
        reads from stdin, writes output to specified file
    accss -o <out_filename> <filename> [<filename>...]
        minimizes the CSS in <filename> and writes output to specified file
         multiple files while be combined to one output file
    accss -r
    accss --restructure-off
        turns structure minimization off
    accss -h
    accss --help
        shows usage information
    accss -v
    accss --version
        shows the version number
    accss -s
    accss --stats
        prints cpmpression statistic to stdout if outfile is specified
    accss -p
    accss --preserve-splitted
        keep splitted shorthand propertys

        before compression
            .mt1{ margin-top:0; } .mt2{ margin-top:100px; }.mt1{ margin-left:0; margin-bottom:0; margin-right:0; }
        without option
            .mt2{margin-top:100px}.mt1{margin:0}
        with option
            .mt1{margin-top:0}.other{margin-top:100px}.mt1{margin-left:0;margin-bottom:0;margin-right:0}
        Note: consider restructure your css to .mt1.mt2{margin-top:100px}
    accss -c
    accss --compat
        compatibility to browser versions

        If set, following property/value sets and all below are not compressed/merged to preserve progressive enhancements for older browsers.
        The default value is all, which means maximum compatibility.
        Set this option to none if you don't use progressive enhancements as this increases compression speed and ratio.

        Example:
            none
                before compression
                    .ex{ display:block; display: inline-block; }
                after compression
                    .ex{display:inline-block}
            ie7
                before compression
                    .ex{ display:block; display: inline-block; }
                after compression
                    .ex{display:block;display:inline-block}

        Possible values:
        all
            default setting
        ie7
            display: inline-block
            clip
                comma syntax
        ie8
            background-position:
                background offsets
                http://www.w3.org/TR/css3-background/#background-position
            background / background-image
                multiple backgrounds
                http://www.w3.org/TR/css3-background/#the-background-image
            css3 colors
                http://www.w3.org/TR/css3-color/
                hsl, hsla, rgba
            cursor:
                http://www.w3.org/TR/css3-ui/#cursor
            viewport units
                http://www.w3.org/TR/css3-values/#viewport-relative-lengths
                vw
                vh
            root em units
                http://www.w3.org/TR/css3-values/#font-relative-lengths
        ie9
            calc as unit value
                http://www.w3.org/TR/css3-values/#calc
            viewport units
                vm IE9 only (wrong implementation of vmin)
                vmin
            css gradients
                http://www.w3.org/TR/css3-images/#gradients
                linear-gradient
                radial-gradient
                http://www.w3.org/TR/css3-images/#repeating-gradients
                repeating-linear-gradient
                repeating-radial-gradient
        ie10
            viewport units:
                vmax
        ie11
            cursor:
                zoom-in and zoom-out
        chrome
            text-decoration
                http://www.w3.org/TR/css-text-decor-3/#line-decoration
        future
            image
                http://dev.w3.org/csswg/css-images-3/#image-notation
            image-set
                http://dev.w3.org/csswg/css-images-3/#image-set-notation
        none
            compress/merge everthing
```
