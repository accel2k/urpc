#!/bin/sh


WD=`dirname "${0}"`
WD=`realpath "${WD}"`

INSTALL_PREFIX=""
BUILD_DIR=""
BUILD_TYPE=""
CLEAN_BUILD=0
DIST_CLEAN=0

show_help()
{

cat <<EOF

Usage:
  ${0}: [OPTION...]

Options:
  -h, --help                     Print this help
  --prefix <string>              Install directory prefix (default: /usr)
  --build-dir <string>           Building directory (default: ${WD}/build)
  --build-type <string>          Build type (Release, Debug, default: Release)
  --clean                        Clean build directory
  --dist-clean                   Clean source directory

EOF

}

#
# Check args.
while [ $# -ge 1 ]; do

  option="$1"

  case $option in

    -h|--help)
      show_help
      exit
      ;;

    --prefix)
      INSTALL_PREFIX="$2"
      shift
      ;;

    --build-dir)
      BUILD_DIR="$2"
      shift
      ;;

    --build-type)
      BUILD_TYPE="$2"
      shift
      ;;

    --clean)
      CLEAN_BUILD=1
      ;;

    --dist-clean)
      DIST_CLEAN=1
      CLEAN_BUILD=1
      ;;

    *)
      echo "unknown option: $option"
      ;;

  esac

  shift

done

#
# Check options.
if [ "x${INSTALL_PREFIX}" = "x" ]; then
  INSTALL_PREFIX="/usr"
fi

if [ "x${BUILD_DIR}" = "x" ]; then
  BUILD_DIR="${WD}/build"
fi

if [ "x${BUILD_TYPE}" = "x" ]; then
  BUILD_TYPE="Release"
fi

#
# Building.
if [ ${CLEAN_BUILD} = 0 ]; then
  mkdir -p "${BUILD_DIR}" || exit
  cd "${BUILD_DIR}" || exit
  cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=${BUILD_TYPE} -D CMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} .. || exit
  make || exit
  cat <<EOF

uRPC finished building, to actually install uRPC you need to type:
# cd "$BUILD_DIR"; sudo make install

EOF
fi

#
# Cleaning.
if [ ${CLEAN_BUILD} = 1 ]; then
  if [ -f "${BUILD_DIR}/Makefile" ]; then
    cd "${BUILD_DIR}" || exit
    make clean
  fi
fi

#
# Dist clean.
if [ ${DIST_CLEAN} = 1 ]; then
  cd "${WD}"
  rm -rf "bin"
  rm -rf "doc/documentation"
  rm -rf "${BUILD_DIR}"
fi
