<?php

/**
 * @file
 * This file contains functions used throughout this module.
 *
 *  Some things you may find:
 *    Fetch pages from a book.
 *    Generate Derivatives.
 *    Check for executables.
 *    etc.
 */

/**
 * Gets all the pages in the given book object.
 *
 * @param FedoraObject $object
 *   The book object to fetch the pages from.
 *
 * @return array
 *   All the pages in the given book.
 *
 * @code
 *   array(
 *     'islandora:100' => array(
 *        'label' => 'Page 100',
 *        'page' => '100',
 *        'pid' => 'islandora:100',
 *      )
 *   );
 * @endcode
 */
function islandora_book_get_pages(FedoraObject $object) {
  // @todo Remove old references to old style sequence numbers.
  $query = 'select $object $page $label from <#ri>
            where $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $object->id . '>
            and $object <fedora-model:label> $label
            and ($object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> $page
            or $object <http://islandora.ca/ontology/relsext#isSequenceNumber> $page)
            order by $page';
  $results = $object->repository->ri->query($query);
  $map = function($o) {
    return array(
      'label' => $o['label']['value'],
      'page' => $o['page']['value'],
      'pid' => $o['object']['value'],
    );
  };
  $pages = array_map($map, $results);
  $get_pid = function($o) {
    return $o['pid'];
  };
  $pids = array_map($get_pid, $pages);
  $pages = count($pids) ? array_combine($pids, $pages) : array();
  $sort = function($a, $b) {
    $a = (is_array($a) && isset($a['page'])) ? $a['page'] : 0;
    $b = (is_array($b) && isset($b['page'])) ? $b['page'] : 0;
    if ($a == $b) {
      return 0;
    }
    return ($a < $b) ? -1 : 1;
  };
  uasort($pages, $sort);
  return $pages;
}

/**
 * Gets the page progression to be used in a viewer from the given book.
 *
 * @param FedoraObject $object
 *   The book object to fetch the page progression from.
 *
 * @return string
 *   The page progression of the book either left to right 'lr' or
 *   right to left 'rl'.
 */
function islandora_book_get_page_progression(FedoraObject $object) {
  $query = 'select $page_progression from <#ri> where <info:fedora/' . $object->id . '>
            <fedora-rels-ext:hasPageProgression> $page_progression';
  $results = $object->repository->ri->query($query);
  if (count($results) > 0) {
    return $results[0]['page_progression']['value'];
  }
  return 'lr';
}

/**
 * Gets the available languages that tesseract supports.
 *
 * @return array
 *   The supported languages where the key is the value tesseract recognizes and
 *   the value is a human readable version of the language.
 */
function islandora_book_get_tesseract_languages() {
  // @todo Generate these values by looking at the tesseract install location.
  return array(
    'eng' => t('English'),
    'fra' => t('French'),
    'deu-frak' => t('German'),
    'por' => t('Portugese'),
    'spa' => t('Spanish'),
    'hin' => t('Hindi'),
    'jpn' => t('Japanese'),
    'ita' => t('Italian'),
  );
}

/**
 * Gets only the enabled languages as defined by the user in the admin form.
 *
 * @return array
 *   The enabled languages where the key is the value tesseract recognizes and
 *   the value is a human readable version of the language.
 */
function islandora_book_get_enabled_tesseract_languages() {
  $tesseract_languages = islandora_book_get_tesseract_languages();
  $enabled_languages = array_filter(variable_get('islandora_book_tesseract_enabled_languages', $tesseract_languages));
  return array_intersect_key($tesseract_languages, $enabled_languages);
}

/**
 * Gets only the enabled ingest derivatives.
 *
 * As defined by the user in the admin form.
 *
 * @return array
 *   The enabled ingest derivatives.
 */
function islandora_book_get_enabled_ingest_derivatives() {
  $default_derivatives = drupal_map_assoc(array('pdf', 'image', 'ocr'));
  return variable_get('islandora_book_ingest_derivatives', $default_derivatives);
}

/**
 * Gets the source file for the given, derivative datastream.
 *
 * @param AbstractFedoraObject $object
 *   The owner of the datastreams.
 * @param string $dsid
 *   The derived datastream ID.
 *
 * @return string
 *   The path to the source file.
 */
function islandora_book_get_page_derivative_source(AbstractFedoraObject $object, $dsid) {
  if (!islandora_book_can_derive($object, $dsid)) {
    return FALSE;
  }
  $source_dsid = islandora_book_get_page_derivative_source_id($dsid);
  $mime_detector = new MimeDetect();
  $ext = $mime_detector->getExtension($object[$source_dsid]->mimeType);
  $source_file = drupal_realpath("temporary://{$object->id}_{$source_dsid}.{$ext}");
  $object[$source_dsid]->getContent($source_file);
  return $source_file;
}

/**
 * Checks if the given object can derive the given datastream.
 *
 * @param AbstractFedoraObject $object
 *   The object to check.
 * @param string $dsid
 *   The derived datastream ID.
 *
 * @return bool
 *   TRUE if the datastream can be derived, FALSE otherwise.
 */
function islandora_book_can_derive(AbstractFedoraObject $object, $dsid) {
  $can_derive = FALSE;
  if (in_array('islandora:bookCModel', $object->models)) {
    // Doesn't check all pages to make sure they can derive PDF files.
    $can_derive = islandora_book_can_create_pdf();
    $can_derive &= islandora_book_can_combine_pdf();
    $can_derive &= ($dsid == 'PDF');
  }
  elseif (in_array('islandora:pageCModel', $object->models)) {
    $check_function = array(
      'PDF' => 'islandora_book_can_create_pdf',
      'OCR' => 'islandora_book_can_create_ocr',
      'RAW_HOCR' => 'islandora_book_can_create_ocr',
      'ENCODED_OCR' => 'islandora_book_can_create_ocr',
      'TN' => 'islandora_book_can_create_images',
      'JP2' => 'islandora_book_can_create_images',
      'JPEG' => 'islandora_book_can_create_images',
    );
    $can_derive = isset($check_function[$dsid]) ? $check_function[$dsid]() : FALSE;
    $can_derive &= islandora_book_page_has_derivative_source($object, $dsid);
  }
  return $can_derive;
}

/**
 * Checks if it is possible to create PDF files with imagemagick.
 *
 * @return bool
 *   TRUE if it is possible, FALSE otherwise.
 */
function islandora_book_can_create_pdf() {
  // @todo Should also check if imagemagick is configured correctly.
  return module_exists('imagemagick');
}

/**
 * Checks if it is possible to combined PDF files with GhostScript.
 *
 * @return bool
 *   TRUE if it is possible, FALSE otherwise.
 */
function islandora_book_can_combine_pdf() {
  $gs = variable_get('islandora_book_gs', '/usr/bin/gs');
  return is_executable($gs);
}

/**
 * Checks if it is possible to create files with tesseract.
 *
 * @return bool
 *   TRUE if it is possible, FALSE otherwise.
 */
function islandora_book_can_create_ocr() {
  $tesseract = variable_get('islandora_book_tesseract', '/usr/bin/tesseract');
  return is_executable($tesseract);
}

/**
 * Check if preprocessing images for OCR is possible on this computer.
 *
 * @return bool
 *   TRUE if preprocessing images is possible, FALSE otherwise.
 */
function islandora_book_can_preprocess_ocr() {
  $gimp = variable_get('islandora_book_gimp', '/usr/bin/gimp');
  return is_executable($gimp);
}

/**
 * Checks if it is possible to create image files.
 *
 * @return bool
 *   TRUE if it is possible, FALSE otherwise.
 */
function islandora_book_can_create_images() {
  return module_exists('islandora_large_image');
}

/**
 * Checks if it is possible to update a book's thumbnail image.
 */
function islandora_book_can_update_book_thumbnail(FedoraObject $object) {
  $pages = islandora_book_get_pages($object);
  if (count($pages)) {
    $page = array_shift($pages);
    $page = islandora_object_load($page['pid']);
    return isset($page['TN']);
  }
  return FALSE;
}

/**
 * Checks if the given derivative datastream's intended source exists.
 *
 * @param AbstractFedoraObject $object
 *   The object to check.
 * @param string $dsid
 *   The derived datastream ID.
 *
 * @return bool
 *   TRUE if the source datastream exists, FALSE otherwise.
 */
function islandora_book_page_has_derivative_source(AbstractFedoraObject $object, $dsid) {
  $source_dsid = islandora_book_get_page_derivative_source_id($dsid);
  return isset($object[$source_dsid]);
}

/**
 * Gets the datastream ID of the intended source for the given derivative.
 *
 * @param string $dsid
 *   The derived datastream ID.
 *
 * @return string
 *   The datastream ID of the source for the given derivative.
 */
function islandora_book_get_page_derivative_source_id($dsid) {
  // @todo Change TIFF to OBJ, when done testing.
  $source_datastreams = array(
    'PDF' => 'OBJ',
    'OCR' => 'OBJ',
    'RAW_HOCR' => 'OBJ',
    'ENCODED_OCR' => 'OBJ',
    'TN' => 'OBJ',
    'JPG' => 'OBJ',
    'JP2' => 'OBJ',
  );
  return $source_datastreams[$dsid];
}

/**
 * Creates a PDF derivative for the given Page object.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated from, and added to.
 * @param array $options
 *   The options to be passed to convert.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_page_derive_pdf_datastream(FedoraObject $object, array $options = array('-compress' => 'LZW')) {
  $pdf_file = islandora_book_page_derive_pdf($object, $options);
  if (!$pdf_file) {
    return FALSE;
  }
  $ret = islandora_book_update_datastream($object, $pdf_file, 'PDF');
  file_unmanaged_delete($pdf_file);
  return $ret;
}

/**
 * Creates a PDF derivative from the given Page object and convert options.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated from.
 * @param array $options
 *   The options to be passed to convert.
 *
 * @return string
 *   The absolute path to the derived file if successful, FALSE otherwise.
 */
function islandora_book_page_derive_pdf(FedoraObject $object, array $options = array('-compress' => 'LZW')) {
  if (!islandora_book_can_derive($object, 'PDF')) {
    return FALSE;
  }
  $source_file = islandora_book_get_page_derivative_source($object, 'PDF');
  $pdf_file = islandora_book_convert_to_pdf($source_file, $options);
  file_unmanaged_delete($source_file);
  return $pdf_file;
}

/**
 * Converts the given image to a PDF file, using the given options.
 *
 * @param string $image_file
 *   The absolute path to the image file.
 * @param array $options
 *   The options to be passed to convert.
 *
 * @return string
 *   The absolute path to the generated PDF if successful, FALSE otherwise.
 */
function islandora_book_convert_to_pdf($image_file, array $options) {
  // Convert options into a string.
  $to_string = function(&$o, $k) {
    $o = "$k $o";
  };
  array_walk($options, $to_string);
  $options = implode(' ', $options);
  $pdf_file = "$image_file.pdf";
  $command_args = "{$options} {$image_file} {$pdf_file}";
  $output = array(); $ret = 0;
  if (_imagemagick_convert_exec($command_args, $output, $ret) !== TRUE) {
    $message = 'imagemagick convert failed to create pdf<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array(
      '@ret' => $ret,
      '@command' => "convert $command_args",
      '@output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $pdf_file;
}

/**
 * Appends a number of PDF files onto the given PDF file.
 *
 * @param array $file
 *   The PDF files to append onto.
 * @param array $files
 *   The PDF files to append.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_pdf_append($file, array $files) {
  $temp_file = "$file.temp.pdf";
  copy($file, $temp_file);
  array_unshift($files, $temp_file);
  $ret = islandora_book_combined_pdf($files, $file);
  file_unmanaged_delete($temp_file);
  return $ret;
}

/**
 * Combines the given PDF files into one output file.
 *
 * @param array $files
 *   The PDF files to be combined in order.
 * @param string $out
 *   The absolute path to the combined PDF file.
 *
 * @return bool
 *   TRUE on success, FALSE otherwise.
 */
function islandora_book_pdf_combine(array $files, $out) {
  $gs = variable_get('islandora_book_gs', '/usr/bin/gs');
  $files = implode(' ', $files);
  $command = "{$gs} -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile={$out} {$files}";
  $output = array(); $ret = 0;
  exec($command, $output, $ret);
  if ($ret != 0) {
    $message = 'gs failed to combined PDF<br/>Error: @ret<br/>Command: @command <br/>Output !output';
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '!output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return TRUE;
}

/**
 * Creates and adds all the image derivatives for the given Page object.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated for.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_page_derive_image_datastreams(FedoraObject $object) {
  module_load_include('inc', 'islandora_large_image', 'includes/islandora_large_image.process');
  return islandora_large_image_create_all_derivatives($object);
}

/**
 * Creates and adds the OCR, RAW_HOCR, and ENCODED_OCR datastreams.
 *
 * @param FedoraObject $object
 *   The page object that the derivatives will be generated for.
 * @param array $options
 *   The options for tesseract/gimp.
 *
 * @return bool
 *   TRUE on success, FALSE otherwise.
 */
function islandora_book_page_derive_ocr_datastreams(FedoraObject $object, array $options = NULL) {
  $rels_ext = $object->relationships;
  $options = isset($options) ? $options : array(
    'language' => islandora_book_get_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'hasLanguage', 'eng'),
    'preprocess' => islandora_book_get_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'preprocess', 'false') == 'true',
  );
  $source_file = islandora_book_get_page_derivative_source($object, 'OCR');
  $source_file = $options['preprocess'] ? islandora_book_ocr_preprocess_image($source_file) : $source_file;
  $ocr_file = islandora_book_convert_to_ocr($source_file, $options);
  $hocr_file = islandora_book_convert_to_hocr($source_file, $options);
  $encoded_hocr_file = $hocr_file ? islandora_book_encode_hocr($hocr_file) : FALSE;
  $ret = $ocr_file && islandora_book_update_datastream($object, $ocr_file, 'OCR');
  $ret &= $hocr_file && islandora_book_update_datastream($object, $hocr_file, 'RAW_HOCR');
  $ret &= $encoded_hocr_file && islandora_book_update_datastream($object, $encoded_hocr_file, 'ENCODED_OCR');
  $language = $options['language'];
  $preprocess = $options['preprocess'] ? 'true' : 'false';
  $rels_ext = $object->relationships;
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'hasLanguage', $language, TRUE);
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'preprocess', $preprocess, TRUE);
  $files = array_filter(array($source_file, $hocr_file, $hocr_file, $encoded_hocr_file));
  array_map('file_unmanaged_delete', $files);
  return $ret;
}

/**
 * Gets the source file for OCR and preprocesses the source file.
 *
 * @param string $source_file
 *   The absolute path to the source image file.
 *
 * @return string
 *   A absolute path to the preprocessed file if successful, if unsuccessful the
 *   unpreprocessed file is returned.
 */
function islandora_book_ocr_preprocess_image($source_file) {
  $executable = variable_get('islandora_book_gimp', '/usr/bin/gimp');
  $dest_file = "$source_file.png";
  $command = "{$executable} -d -i -b '(munge-typeset-ocr {$source_file} {$dest_file} 5.0 10.0 0.0 200.0 5.0)' -b '(gimp-quit 0)'";
  $output = array(); $ret = 0;
  exec($command, $output, $ret);
  // Sometimes Gimp fails but returns success anyways so we check for the
  // destination files existance.
  if ($ret != '0' || !file_exists($dest_file)) {
    $message = 'gimp failed to preproces image<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '@output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return $source_file;
  }
  return $dest_file;
}

/**
 * Converts the given image to a plain text OCR file, using the given options.
 *
 * @param string $image_file
 *   The absolute path to the image file.
 * @param array $options
 *   The options to be passed to tesseract.
 *
 * @return string
 *   The absolute path to the generated OCR if successful, FALSE otherwise.
 */
function islandora_book_convert_to_ocr($image_file, array $options) {
  $out_file = "$image_file.txt";
  $executable = variable_get('islandora_book_tesseract', '/usr/bin/tesseract');
  $output = array(); $ret = 0;
  $command = "{$executable} {$image_file} {$image_file} -l {$options['language']} 2>&1";
  exec($command, $output, $ret);
  if ($ret != '0') {
    $message = 'tesseract failed to create ocr<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '@output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $out_file;
}

/**
 * Converts the given image to a html HOCR file, using the given options.
 *
 * @param string $image_file
 *   The absolute path to the image file.
 * @param array $options
 *   The options to be passed to tesseract.
 *
 * @return string
 *   The absolute path to the generated HOCR if successful, FALSE otherwise.
 */
function islandora_book_convert_to_hocr($image_file, array $options) {
  $out_file = "$image_file.html";
  $executable = variable_get('islandora_book_tesseract', '/usr/bin/tesseract');
  $output = array(); $ret = 0;
  $command = "{$executable} {$image_file} {$image_file} -l {$options['language']} hocr 2>&1";
  exec($command, $output, $ret);
  if ($ret != '0' || !file_exists($out_file)) {
    $message = 'tesseract failed to create hocr<br/>Error: @ret<br/>Command: @command<br/>Output: @output';
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '@output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return $out_file;
}

/**
 * Encodes the given HTML OCR file as XML.
 *
 * @param string $file
 *   The absolute path to the HOCR file.
 *
 * @return string
 *   The absolute path to the encoded HOCR file if successful, FALSE otherwise.
 */
function islandora_book_encode_hocr($file) {
  $proc = new XSLTProcessor();
  // Convert the hocr from html 4.01 transitional to xhtml and lower case
  // the text.
  $html_string = file_get_contents($file);
  $doc_type = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">';
  $xml_string = str_replace($doc_type, '<?xml version="1.0" encoding="UTF-8"?>', $html_string);
  $input = new DOMDocument();
  $input->loadXML($xml_string);
  $path = drupal_get_path('module', 'islandora_book');
  $xsl = new DOMDocument();
  $xsl->load($path . '/data/xsl/hocrToLowerhocr.xsl');
  $xsl = $proc->importStylesheet($xsl);
  $result = $proc->transformToDoc($input);
  // Create new transformed file.
  $file = "$file.xml";
  $result->save($file);
  return $file;
}

/**
 * Updates a the given books thumbnail from the first page in the book.
 *
 * @param FedoraObject $object
 *   The book object to update.
 *
 * @return bool
 *   TRUE on success, FALSE otherwise.
 */
function islandora_book_update_book_thumbnail(FedoraObject $object) {
  if (!islandora_book_can_update_book_thumbnail($object)) {
    return FALSE;
  }
  $pages = islandora_book_get_pages($object);
  $page = array_shift($pages);
  $page = islandora_object_load($page['pid']);
  $mime_detector = new MimeDetect();
  $ext = $mime_detector->getExtension($page['TN']->mimeType);
  $file = drupal_realpath('temporary://{$page->id}_TN.{$ext}');
  $page['TN']->getContent($file);
  $ret = islandora_book_update_datastream($object, $file, 'TN', 'Thumbnail');
  file_unmanaged_delete($file);
  return $ret;
}

/**
 * Gets the given relationship if one is defined.
 *
 * Assumes only one relationship matches the given parameters.
 * Will return the given default if no relationship is found.
 *
 * @param FedoraRelationships $relationships
 *   The Fedora relationships to check, either RELS-EXT or RELS-INT.
 * @param string $predicate_uri
 *   The predicate URI.
 * @param string $predicate
 *   The predicate.
 * @param mixed $default
 *   The default value to return if no relationship was found.
 *
 * @return mixed
 *   The first matching relationships value if found, otherwise the given
 *   default it returned.
 */
function islandora_book_get_relationship(FedoraRelationships $relationships, $predicate_uri, $predicate, $default = NULL) {
  $results = $relationships->get($predicate_uri, $predicate);
  return count($results) > 0 ? $results[0]['object']['value'] : $default;
}

/**
 * Sets a relationship removing any previous relationships.
 *
 * @param FedoraRelationships $relationships
 *   The Fedora relationships to be modified, either RELS-EXT or RELS-INT.
 * @param string $predicate_uri
 *   The predicate URI.
 * @param string $predicate
 *   The predicate.
 * @param string $object
 *   The object.
 * @param bool $literal
 *   TRUE if the object is a literal, FALSE if it is an object.
 */
function islandora_book_set_relationship(FedoraRelationships $relationships, $predicate_uri, $predicate, $object, $literal = FALSE) {
  $relationships->remove($predicate_uri, $predicate, NULL, $literal);
  $relationships->add($predicate_uri, $predicate, $object, $literal);
}

/**
 * Create/Update a datastream with the given parameters.
 *
 * @param AbstractFedoraObject $object
 *   The object to update.
 * @param string $file
 *   The absolute path to the file to use as content for the datastream.
 * @param string $dsid
 *   The datastream ID.
 * @param string $label
 *   The datastream label.
 * @param string $mimetype
 *   The datastream mimetype, if none given it will be guessed from the file
 *   extension.
 * @param string $control_group
 *   The datastream control group.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_update_datastream(AbstractFedoraObject $object, $file, $dsid, $label = NULL, $mimetype = NULL, $control_group = 'M', $copy = TRUE) {
  $mime_detector = new MimeDetect();
  if (empty($object[$dsid])) {
    $ds = $object->constructDatastream($dsid, $control_group);
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object[$dsid];
  }
  $ds->setContentFromFile($file, $copy);
  $ds->label = isset($label) ? $label : $dsid;
  $ds->mimetype = isset($mimetype) ? $mimetype : $mime_detector->getMimetype($file);
  return TRUE;
}
