# Book Solution Pack [![Build Status](https://travis-ci.org/Islandora/islandora_solution_pack_book.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_solution_pack_book)

## Introduction

Allows for the creation of book and page object (islandora:bookCModel,islandora:pageCModel). Books are essentially a collection Pages, containing only metadata and optionally a PDF file of the book. A Book's PDF can be generated from each page. Pages are based on an uploaded tiff of the page. From the uploaded TIFF its possible to generate images for use in the "Islandora Internet Archive Bookreader". It is also possible to generate PDF files per page. OCR and OCR coordinate data can also be generated from the uploaded TIFF.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Paged Content](https://github.com/Islandora/islandora_paged_content)
* [Islandora Large Image Solution Pack](https://github.com/Islandora/islandora_solution_pack_large_image)
* [Islandora OCR](https://github.com/Islandora/islandora_ocr)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Select configuration options for page derivatives, Parent Solr Field, and select a viewer for the book objec and page objects in Administration » Islandora » Book Collection (admin/islandora/book).

![Configuration](http://i.imgur.com/7ICJfeZ.png)

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors
Current maintainers:

* [Alan Stanley](https://github.com/ajstanley)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
