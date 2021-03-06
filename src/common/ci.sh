#!/bin/sh
# Copyright Ole-André Rodlie.
#
# ole.andre.rodlie@gmail.com
#
# This software is governed by the CeCILL license under French law and
# abiding by the rules of distribution of free software. You can use,
# modify and / or redistribute the software under the terms of the CeCILL
# license as circulated by CEA, CNRS and INRIA at the following URL
# "https://www.cecill.info".
#
# As a counterpart to the access to the source code and rights to
# modify and redistribute granted by the license, users are provided only
# with a limited warranty and the software's author, the holder of the
# economic rights and the subsequent licensors have only limited
# liability.
#
# In this respect, the user's attention is drawn to the associated risks
# with loading, using, modifying and / or developing or reproducing the
# software by the user in light of its specific status of free software,
# that can mean that it is complicated to manipulate, and that also
# so that it is for developers and experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to test and test the software's suitability
# Requirements in the conditions of their systems
# data to be ensured and, more generally, to use and operate
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license and that you accept its terms.

set -e

SETUP=${SETUP:-1}

if [ "${SETUP}" = 1 ]; then
  sudo chmod 777 /opt
fi

OS=`uname -s`
CWD=`pwd`
MXE=/opt/mxe
SDK=/opt/${OS}2
DEPLOY=/opt/deploy

COMMIT="${TRAVIS_COMMIT}"
if [ "${TRAVIS_TAG}" != "" ]; then
  COMMIT=""
fi

if [ "${SETUP}" = 1 ]; then
  echo "Extracting sdk legal ..."
  wget https://sourceforge.net/projects/prepress/files/sdk/cyan-1.2-sdk-legal.tar.xz/download && mv download download.tar.xz
  tar xf download.tar.xz -C /opt
  rm -f download.tar.xz
  if [ "${OS}" = "Linux" ]; then
    echo "Setup ubuntu ..."
    sudo apt remove --purge imagemagick imagemagick-common
    sudo apt-get update
    sudo apt-get install cmake tree qtbase5-dev libpng-dev libjpeg-dev liblcms2-dev libtiff-dev libbz2-dev zlib1g-dev liblzma-dev
    sudo apt-get install scons autoconf automake autopoint bash bison bzip2 gettext git gperf intltool
    sudo apt-get install openssl patch perl pkg-config python ruby sed unzip wget xz-utils wine
    sudo apt-get install libgdk-pixbuf2.0-dev libltdl-dev libssl-dev libtool libxml-parser-perl make
    sudo apt-get install flex g++ g++-multilib libc6-dev-i386 wine p7zip-full libfreetype6-dev libfontconfig1-dev
    echo "Extracting win64 sdk ..."
    mkdir -p ${MXE}
    wget https://sourceforge.net/projects/prepress/files/sdk/cyan-sdk-20181231-mingw64.tar.xz/download && mv download download.tar.xz
    tar xf download.tar.xz -C ${MXE}/
    rm -f download.tar.xz
    #echo "Update win64 sdk ..."
    #wget https://sourceforge.net/projects/prepress/files/sdk/cyan-sdk-20181226_3-win64-static.tar.xz/download && mv download download.tar.xz
    #rm -rf ${MXE}/usr/x86_64-w64-mingw32.static
    #tar xf download.tar.xz -C ${MXE}/
    #rm -f download.tar.xz
    echo "Extracting linux64 sdk ..."
    wget https://sourceforge.net/projects/prepress/files/sdk/cyan-sdk-20181226-linux64.tar.xz/download && mv download download.tar.xz
    tar xf download.tar.xz -C /opt
    rm -f download.tar.xz
  elif [ "${OS}" = "Darwin" ]; then
    curl -L https://sourceforge.net/projects/prepress/files/sdk/cyan-1.2-sdk-mac11clang6.tar.xz/download --output download.tar.xz
    tar xf download.tar.xz -C /opt
    rm -f download.tar.xz
    curl -L https://sourceforge.net/projects/prepress/files/sdk/cyan-sdk-20181226-mac11.tar.xz/download --output download.tar.xz
    tar xf download.tar.xz -C /opt
    rm -f download.tar.xz
  fi
fi

if [ "${OS}" = "Linux" ]; then
  export PATH="${SDK}/bin:/usr/bin:/bin"
  export PKG_CONFIG_PATH="${SDK}/lib/pkgconfig"

  echo "==> Building regular CI ..."
  mkdir -p $CWD/ci1
  cd $CWD/ci1
  qmake PREFIX=/usr CONFIG+=deploy ..
  make
  #make test
  make INSTALL_ROOT=`pwd`/pkg install
  tree pkg
  mkdir -p $CWD/ci2
  cd $CWD/ci2
  qmake PREFIX=/usr CONFIG+=deploy CONFIG+=with_ffmpeg ..
  make
  #make test
  make INSTALL_ROOT=`pwd`/pkg install
  tree pkg

  cd ${CWD}
  echo "===> Building for Linux64 ..."
  mkdir -p ${CWD}/linux64 && cd ${CWD}/linux64
  qmake GIT=${COMMIT} CONFIG+=release PREFIX=/usr CONFIG+=deploy ..
  make
  #make test
  strip -s src/build/Cyan
  mv src/build/Cyan .

  echo "===> Building win64 ..."
  mkdir -p ${CWD}/win64
  cd ${CWD}/win64
  TARGET=x86_64-w64-mingw32.static
  MINGW="${MXE}/usr/${TARGET}"
  CMAKE="${TARGET}-cmake"
  QT=${MINGW}/qt5
  QMAKE=${QT}/bin/qmake
  STRIP="${MXE}/usr/bin/${TARGET}-strip"
  PATH="${MXE}/usr/bin:/usr/bin:/bin"
  PKG_CONFIG_PATH="${MINGW}/lib/pkgconfig"
  ${QMAKE} GIT=${COMMIT} CONFIG+=release CONFIG+=deploy ..
  make
  #make test
  ${STRIP} -s src/build/Cyan.exe
  mv src/build/Cyan.exe .
elif [ "${OS}" = "Darwin" ]; then
  echo "===> Building mac64 ..."
  PKG_CONFIG=${SDK}/bin/pkg-config
  PKG_CONFIG_PATH="${SDK}/lib/pkgconfig:${PKG_CONFIG_PATH}"
  PATH=${SDK}/bin:/usr/bin:/bin
  mkdir -p ${CWD}/mac64 && cd ${CWD}/mac64
  qmake GIT=${COMMIT} CONFIG+=release CONFIG+=deploy ..
  make
  #make test
  MP=/opt/local/lib/libomp/libomp.dylib
  cp ${MP} src/build/Cyan.app/Contents/MacOS/
  install_name_tool -change ${MP} @executable_path/libomp.dylib src/build/Cyan.app/Contents/MacOS/Cyan
  install_name_tool -id @executable_path/libomp.dylib src/build/Cyan.app/Contents/MacOS/libomp.dylib
  strip -u -r src/build/Cyan.app/Contents/MacOS/*
  mv src/build/Cyan.app .
fi

echo "===> Creating archives ..."
mkdir -p ${CWD}/deploy ${DEPLOY} && cd ${CWD}/deploy
TAG=`date "+%Y%m%d%H%M"`
if [ "${TRAVIS_TAG}" != "" ]; then
  echo "===> RELEASE MODE"
  TAG="${TRAVIS_TAG}"
fi

if [ "${OS}" = "Linux" ]; then
  mkdir -p Cyan-${TAG}-Windows/third-party Cyan-${TAG}-Linux/third-party
  cp ${CWD}/docs/LICENSE.CeCILLv21 Cyan-${TAG}-Windows/
  cp ${CWD}/docs/LICENSE.CeCILLv21 Cyan-${TAG}-Linux/
  cp -a /opt/legal/Windows/* Cyan-${TAG}-Windows/third-party/
  cp -a /opt/legal/Linux/* Cyan-${TAG}-Linux/third-party/
  cp ${CWD}/win64/Cyan.exe Cyan-${TAG}-Windows/
  cp ${CWD}/linux64/Cyan Cyan-${TAG}-Linux/
  cp ${CWD}/src/share/cyan.desktop Cyan-${TAG}-Linux/
  cp -a ${CWD}/src/share/icons/hicolor/128x128/apps/cyan.png Cyan-${TAG}-Linux/
  7za -mx=9 a -r Cyan-${TAG}-Windows.7z Cyan-${TAG}-Windows
  WIN_CHECKSUM=`sha256sum Cyan-${TAG}-Windows.7z | awk '{print $1}'`
  cp Cyan-${TAG}-Windows.7z ${DEPLOY}/
  echo "===> Windows checksum ${WIN_CHECKSUM}"
  tar cvvf Cyan-${TAG}-Linux.tar Cyan-${TAG}-Linux
  xz -9 Cyan-${TAG}-Linux.tar
  mv Cyan-${TAG}-Linux.tar.xz Cyan-${TAG}-Linux.txz
  cp Cyan-${TAG}-Linux.txz ${DEPLOY}/
  LIN_CHECKSUM=`sha256sum Cyan-${TAG}-Linux.txz | awk '{print $1}'`
  echo "===> Linux checksum ${LIN_CHECKSUM}"
elif [ "${OS}" = "Darwin" ]; then
  mkdir -p Cyan-${TAG}-Mac/third-party
  cp ${CWD}/docs/LICENSE.CeCILLv21 Cyan-${TAG}-Mac/
  cp -a /opt/legal/Mac/* Cyan-${TAG}-Mac/third-party/
  cp -a ${CWD}/mac64/Cyan.app Cyan-${TAG}-Mac/
  hdiutil create -volname "Cyan ${TAG}" -srcfolder Cyan-${TAG}-Mac -ov -format UDBZ Cyan-${TAG}-Mac.dmg
  cp Cyan-${TAG}-Mac.dmg ${DEPLOY}/
  MAC_CHECKSUM=`shasum -a 256 Cyan-${TAG}-Mac.dmg | awk '{print $1}'`
  echo "===> Mac checksum ${MAC_CHECKSUM}"
fi

if [ "${TRAVIS_PULL_REQUEST}" != "false" ] && [ "${TRAVIS_PULL_REQUEST}" != "" ]; then
    echo "===> Uploading archives to transfer.sh ..."
    if [ "${OS}" = "Linux" ]; then
      UPLOAD_WIN=`curl --upload-file ./Cyan-${TAG}-Windows.7z https://transfer.sh/Cyan-${TAG}-Windows.7z`
      UPLOAD_LIN=`curl --upload-file ./Cyan-${TAG}-Linux.txz https://transfer.sh/Cyan-${TAG}-Linux.txz`
      echo "===> Windows snapshot ${UPLOAD_WIN}"
      echo "===> Linux snapshot ${UPLOAD_LIN}"
      if [ "${UPLOAD_WIN}" != "" ] && [ "${UPLOAD_LIN}" != "" ]; then
          COMMENT="**CI for this pull request:** Windows build is available at ${UPLOAD_WIN} with SHA256 checksum ${WIN_CHECKSUM}. Linux build is available at ${UPLOAD_LIN} with SHA256 checksum ${LIN_CHECKSUM}."
          curl -H "Authorization: token ${GITHUB_TOKEN}" -X POST -d "{\"body\": \"${COMMENT}\"}" "https://api.github.com/repos/${TRAVIS_REPO_SLUG}/issues/${TRAVIS_PULL_REQUEST}/comments"
      fi
    elif [ "${OS}" = "Darwin" ]; then
      UPLOAD_MAC=`curl --upload-file ./Cyan-${TAG}-Mac.dmg https://transfer.sh/Cyan-${TAG}-Mac.dmg`
      echo "===> Mac snapshot ${UPLOAD_MAC}"
      if [ "${UPLOAD_MAC}" != "" ]; then
        COMMENT="**CI for this pull request:** Mac build is available at ${UPLOAD_MAC} with SHA256 checksum ${MAC_CHECKSUM}."
        curl -H "Authorization: token ${GITHUB_TOKEN}" -X POST -d "{\"body\": \"${COMMENT}\"}" "https://api.github.com/repos/${TRAVIS_REPO_SLUG}/issues/${TRAVIS_PULL_REQUEST}/comments"
      fi
    fi
fi

