#!/bin/bash

if [[ $TRAVIS_PHP_VERSION == '5.3.3' ]]; then
  mkdir ~/tesseract
  cd ~/tesseract
  wget http://alpha.library.yorku.ca/tesseract.tar.gz
  wget http://alpha.library.yorku.ca/tessdata.tar.gz
  tar xf tesseract.tar.gz && rm -rf tesseract.tar.gz
  dpkg -i tesseract/libleptonica.deb
  dpkg -i tesseract/tesseract-ocr.deb
  tar xf tessdata.tar.gz && rm -rf tessdata.tar.gz
  cp tessdata/* /usr/local/share/tessdata/
  sudo ldconfig
  echo -e "\ntesseract output:"
else
  sudo apt-get -y install tesseract-ocr tesseract-ocr-eng tesseract-ocr-fra
fi

tesseract --version && tesseract --list-langs
