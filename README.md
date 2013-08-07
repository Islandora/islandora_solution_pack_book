BUILD STATUS
------------
Current build status:
[![Build Status](https://travis-ci.org/Islandora/islandora_solution_pack_book.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_solution_pack_book)

CI Server:
http://jenkins.discoverygarden.ca

Book Solution Pack
=========================

CONTENTS OF THIS FILE
---------------------

 * summary
 * requirements
 * installation
 * configuration
 * customization
 * troubleshooting
 * faq
 * contact
 * sponsors


SUMMARY
-------

Book Solution Pack

This allows for the creation of book and page object (islandora:bookCModel,
islandora:pageCModel). Books are essentially a collection Pages, containing
only metadata and optionally a PDF file of the book. A Book's PDF can be
generated from each page. Pages are based on an uploaded tiff of the page.
From the uploaded TIFF its possible to generate images for use in the
"Islandora Internet Archive Bookreader". It is also possible to generate PDF
files per page. OCR and OCR coordinate data can also be generated from the
uploaded TIFF.

REQUIREMENTS
------------

  * Islandora
  * Islandora Paged Content

INSTALLATION
------------

Tesseract:

Tesseract is an OCR engine that was developed at HP Labs between 1985 and 1995
it is currently being developed at Google. Recognized as one of the most
accurate open source OCR engines available, Tesseract will read binary, grey, or
colour images and output text.

A TIFF reader that will read uncompressed TIFF images is also included.
Islandora Book Solution Pack currently uses Tesseract version 3.2, which can be
obtained from the project home page. Lower versions are not supported.

http://code.google.com/p/tesseract-ocr/

Installation will differ depending on your operating system; please see the
README Wiki for detailed instructions.

http://code.google.com/p/tesseract-ocr/wiki/ReadMe

Gimp:

GIMP is the GNU Image Manipulation Program. It is a freely distributed piece of
software for such tasks as photo retouching, image composition and image
authoring. It works on many operating systems, in many languages.

Islandora Book Solution Pack currently uses Gimp version 2.6, which can be
obtained from the project home page.

http://www.gimp.org/downloads/install_help.html

Large Image Solution Pack:

Please see the documentation included with the Large Image Solution Pack.

Ghost Script:

Typically comes with ImageMagick, which is a requirement of the Large Image
Solution Pack. See the documentation include with it for more details.


CONFIGURATION
-------------


CUSTOMIZATION
-------------


TROUBLESHOOTING
---------------


F.A.Q.
------

CONTACT
-------


SPONSORS
--------
