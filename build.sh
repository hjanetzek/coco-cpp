#!/bin/sh
#------------------------------------------------------------------------------
usage() {
    while [ "$#" -ge 1 ]; do echo "$1"; shift; done
    cat<<USAGE

usage: ${0##*/} [OPTION]
options:
  -parser        create new parser code first
  -warn          enable additional gcc warnings

* compile Coco executable

USAGE
    exit 1
}
#------------------------------------------------------------------------------
cd ${0%/*} || exit 1    # run from this directory

unset warn

# parse options
while [ "$#" -gt 0 ]
do
    case "$1" in
    -h | -help)
        usage
        ;;
    -parser)
        ./build-parser.sh
        ;;
    -warn)
        warn="-Wno-strict-aliasing -Wextra -Wno-unused-parameter -Wold-style-cast -Wnon-virtual-dtor"
        ;;
    *)
        usage "unknown option/argument: '$*'"
        ;;
    esac
    shift
done


echo "compile Coco executable"
echo "~~~~~~~~~~~~~~~~~~~~~~~"
echo "g++ *.cpp -o Coco -g -Wall $warn"
echo

g++ *.cpp -o Coco -g -Wall $warn

if [ $? -eq 0 ]
then
    echo
    echo "done"
    echo
else
    echo
    echo "errors detected in compilation"
    echo
fi

# ----------------------------------------------------------------- end-of-file
