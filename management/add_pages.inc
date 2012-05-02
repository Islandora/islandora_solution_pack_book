<?php

function book_add_pages(&$form_state, $pid) {
  $form['page_add'] = array(
    '#attributes' => array('class' => 'page_fieldset'),
    '#type' => 'fieldset',
    '#collapsed' => FALSE,
    '#collapsible' => true,
      //'#title' => t('Add pages to this book'),
      //'#description' => t('Add pages to this book object.'),
  );
  $form['page_add']['link'] = array(
    '#value' => l(t("Append or Insert Pages"), "islandora/book/upload/$pid") . '<br />',
  );
  $form['page_add']['bd_link'] = array(
    '#value' => l(t("Create Digital Page"), "islandora/book/borndigital/$pid"),
  );

  return $form;
}

/**
 * book add pages form
 * @param array $form_state
 * @param string $pid
 * @param string $page_cmodel
 * @param string $relation
 * @return array
 */
function book_add_pages_form(&$form_state, $pid, $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOf') {
  $file = (isset($form_state['values']['ingest-file-location']) ? $form_state['values']['ingest-file-location'] : '');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $book_title = $item->objectProfile->objLabel;

  $prompt = t("Add pages to @title", array('@title' => $book_title));
  $submit_prompt = t('Ingest Pages');
  if ($page_cmodel == 'islandora:bd_pageCModel') {
    $prompt = t("Create digital page for @title", array('@title' => $book_title));
    $submit_prompt = t("Create page");
  }
  $pages = TRUE;
  $current_page_count = count(get_page_pids($pid));
  if ($current_page_count == 0) {
    $pages = FALSE;
  }
  $message = t("This book currently has @count pages, new pages will be appended by default. !break", array("@count" => $current_page_count, '!break' => '<br />'));
  $message .= t("Choose a number lower than @count to insert the new pages at the chosen location", array("@count" => $current_page_count, '!break' => '<br />'));

  $form['page_add'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => false,
    '#title' => $prompt,
    '#description' => t('Add page content to this book.'),
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['book_title'] = array(
    '#type' => 'hidden',
    '#value' => $book_title,
  );
  $form['page_cmodel'] = array(
    '#type' => 'hidden',
    '#value' => $page_cmodel,
  );

  $form['page_count'] = array(
    '#type' => 'hidden',
    '#value' => $current_page_count,
  );

  $form['relation'] = array(
    '#type' => 'hidden',
    '#value' => $relation,
  );
  if ($pages) {
    $form['page_add']['insertion_point'] = array(
      '#type' => 'textfield',
      '#title' => t('Previous Page number'),
      '#default_value' => $current_page_count,
      '#description' => $message,
      '#size' => 5,
    );
  }
  $form['page_add']['upload_options'] = array(
    '#title' => "<strong>Access zipped page images</strong>",
    '#description' => t('Select either ftp file location on the server, or upload zipped page images, but not both.'),
  );


  $form['page_add']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 10,
    '#value' => $submit_prompt,
  );
  return $form;
}

/**
 * book add pages form validate
 * @param array $form
 * @param array $form_state
 */
function book_add_pages_form_validate($form, &$form_state) {
  $_SESSION['manage_chosen'] = TRUE;
  if (isset($form_state['values']['insertion_point'])) {
    $insertion_point = $form_state['values']['insertion_point'] + 0;
    $count = $form_state['values']['page_count'] + 0;
    if (!is_int($insertion_point) || $insertion_point > $count || $insertion_point < 0) {
      form_set_error('insertion_point', "Number must be a positive integer less than or equal to $count");
      return;
    }
  }

  unset($_SESSION['manage_chosen']);
}

/**
 * book add pages form submit
 * @param array $form
 * @param array $form_state
 */
function book_add_pages_form_submit($form, &$form_state) {
  global $base_path;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pageCModel = $form_state['values']['page_cmodel'];
  $pid = $form_state['values']['pid'];
  $start_page = (isset($form_state['values']['insertion_point'])) ? $form_state['values']['insertion_point'] + 1 : 1;
  $dirName = str_replace(':', '_', $pid);
  $tmpDir = file_directory_path() . '/' . $dirName . '/';
  $relationship = $form_state['values']['relation'];
  if ($pageCModel == 'islandora:pageCModel') {
    $file_list = get_page_files($pid);
    ibook_create_page_objects($pid, $file_list, $form_state['values']['page_cmodel'], $relationship, $tmpDir, $start_page);
  }
  if ($pageCModel == 'islandora:bd_pageCModel') {
    renumber_pages($start_page, $pid, 1);
    $path = drupal_get_path('module', 'islandora_book') . '/images/';


    $book_title = $form_state['values']['book_title'];
    $page_title = "$book_title - Page " . sprintf("%03d", $start_page);
    $parts = explode(':', $pid);
    $nextPid = FEDORA_ITEM::get_next_PID_in_namespace($parts[0]);
    $page_item = Fedora_Item::ingest_new_item($nextPid, 'A', $page_title);
    $page_item->add_relationship('hasModel', $pageCModel, FEDORA_MODEL_URI);
    $page_item->add_relationship('isPageNumber', sprintf("%04d", $start_page), ISLANDORA_PAGE_URI);
    $page_item->add_relationship('isPageOf', $pid, ISLANDORA_PAGE_URI);
    $page_item->add_relationship($relationship, $pid);
    $page_item->add_datastream_from_file($path . 'Borndigital_tn.png', 'TN', 'Thumbnail', 'image/png', 'M');
    $page_item->add_datastream_from_file($path . 'Borndigital_Fullsize.jpg', 'JPEG', 'JPEG Image', 'image/jpg', 'M');
    $page_item->add_datastream_from_string('Born digital - not yet edited', 'OCR', 'OCR', 'text/plain', 'M');
    drupal_goto("fedora/repository/$pid");
  }
}

/**
 * Create a page object for each element in $image_list as a child object of $pid
 * @param string $pid
 * @param array $image_list
 * @param string $page_cmodel
 * @param string $relation
 * @param string $tmpDir
 * @return boolean
 */
function ibook_create_page_objects($pid, $image_list = array(), $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOfCollection', $tmpDir, $start_page) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/ObjectHelper');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $book = new Fedora_Item($pid);
  if (!$book->exists()) {
    return FALSE;
  }
  $dirName = str_replace(':', '_', $pid);
  $page_pids = array();
  $image_count = count($image_list);
  $batch = array(
    'title' => "Creating $image_count page objects and book PDF.",
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/management/add_pages.inc',
    'init_message' => t('Preparing data...'),
    'progress_message' => t('@current of @total tasks completed.'),
  );
  $page_num = $start_page;
  $book_title = $book->objectProfile->objLabel;
  $batch['operations'][] = array('renumber_pages', array($start_page, $pid, $image_count));
  foreach ($image_list as $image_path) {
    $batch['operations'][] = array('ibook_add_single_page_object', array($pid, $image_path, $page_num, "$book_title - Page " . sprintf("%03d", $page_num), $page_cmodel, $relation));
    $page_num++;
  }
  if (variable_get('islandora_create_book_pdf', FALSE)) {
    $batch['operations'][] = array('makePDF', array($pid, $image_list, $tmpDir));
  }
  $batch['operations'][] = array('  deleteDirectory($tmpDir);', array($tmpDir));
  $collection = get_collection_from_pid($pid);
  batch_set($batch);
  batch_process("fedora/repository/$pid");
  $objectHelper = new ObjectHelper;
  $dc_xml = $objectHelper->getStream($pid, 'DC');
  if (!preg_match('/<dc:type>ingested<\/dc:type>/', $dc_xml)) {
    $xml = new DOMDocument();
    $xml->loadXML($dc_xml);
    $root = $xml->documentElement;
    $typeNode = $xml->createElement('dc:type', 'ingested');
    $typeNode = $xml->createElementNS("http://purl.org/dc/elements/1.1/", 'dc:type', 'ingested');
    $root->appendChild($typeNode);
    $book->modify_datastream_by_value($xml->saveXML(), 'DC', 'Dublin core', 'text/xml', TRUE, "Added Type of Ingested", FALSE);
  }
  $collection = get_collection_from_pid($pid);
  drupal_goto('fedora/repository/' . $collection[0]);
}

/**
 * ibook add single page object
 * @param string $book_pid
 * @param string $page_file
 * @param int $page_num
 * @param string $page_title
 * @param string $page_cmodel
 * @param string $relation
 */
function ibook_add_single_page_object($book_pid, $page_file, $page_num = 1, $page_title = NULL, $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOf') {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $parts = explode(':', $book_pid);
  $nextPid = FEDORA_ITEM::get_next_PID_in_namespace($parts[0]);
  $page_item = Fedora_Item::ingest_new_item($nextPid, 'A', $page_title);
  $page_item->add_relationship('hasModel', $page_cmodel, FEDORA_MODEL_URI);
  $page_item->add_relationship('isPageNumber', sprintf("%04d", $page_num), ISLANDORA_PAGE_URI);
  $page_item->add_relationship('isPageOf', $book_pid, ISLANDORA_PAGE_URI);
  $page_item->add_relationship($relation, $book_pid);
  // add original tiff
  $page_item->add_datastream_from_file($page_file, 'TIFF', 'Archival TIFF', 'image/tiff', 'M');
  $page_file = preg_replace('/\/\//', '/', $page_file);

  //add compresed jp2 for viewer
  if (variable_get('islandora_book_create_images', TRUE)) {
    exec('kdu_compress -i "' . $page_file . '" -o "' . $page_file . '.jp2' . '" -rate 0.5 Clayers=1 Clevels=7 "Cprecincts={256,256},{256,256},{256,256},{128,128},{128,128},{64,64},{64,64},{32,32},{16,16}" "Corder=RPCL" "ORGgen_plt=yes" "ORGtparts=R" "Cblk={32,32}" Cuse_sop=yes', $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file.jp2", 'JP2', "Compressed JP2", 'image/jp2', 'M');
    }

    //add thumbnail image
    exec("convert {$page_file}[0] -colorspace RGB -thumbnail 120x $page_file-tn.jpg", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
      if ($page_num == 1) {
        $book_item = new Fedora_Item($book_pid);
        $DSIDS = $book_item->get_datastreams_list_as_array();
        if (array_key_exists("TN", $DSIDS)) {
          $book_item->purge_datastream('TN');
        }
        $book_item->add_datastream_from_file("$page_file-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
      }
    }
    //add jpeg
    exec("convert -resize 800 \"{$page_file}\"[0] \"$page_file-med.jpg\"", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file-med.jpg", 'JPEG', "JPEG image", 'image/jpeg', 'M');
    }
  }
  //perform OCR using Tesseract, add text datastream

  if (variable_get('islandora_book_do_ocr', TRUE)) {
    $executable = variable_get('islandora_ocr_path', '/usr/bin/tesseract');
    $returnValue = "";
    $output = "";
    exec("$executable $page_file $page_file -l eng", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file.txt", 'OCR', "Scanned Text", 'text/plain', 'M');
    }
  }
}

/**
 * delete directory
 * @param string $dir
 * @return boolean
 */
function deleteDirectory($dir) {
  if (!file_exists($dir))
    return TRUE;
  if (!is_dir($dir))
    return unlink($dir);
  foreach (scandir($dir) as $item) {
    if ($item == '.' || $item == '..')
      continue;
    if (!deleteDirectory($dir . DIRECTORY_SEPARATOR . $item))
      return FALSE;
  }
  return rmdir($dir);
}

/**
 * make PDF
 * @param string $book_pid
 * @param array $file_list
 * @param string $tmpDir
 */
function makePDF($book_pid, $file_list, $tmpDir) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $book_item = new Fedora_Item($book_pid);
  $outputFile = "${tmpDir}book.pdf";
  $tiffs = array();
  $cmdString = 'convert -compress LZW';
  foreach ($file_list as $tiff) {
    $cmdString .= " $tiff";
  }
  $cmdString .= " $outputFile";
  exec($cmdString, $output, $returnValue);
  $datastreams = $book_item->get_datastreams_list_as_array();
  if ($returnValue == '0') {
    if (array_key_exists('PDF', $datastreams)) {
      $book_item->purge_datastream('PDF');
    }
    $book_item->add_datastream_from_file($outputFile, 'PDF', "PDF of Book", 'application/pdf', 'M');
  }
}

function page_delete(&$form_state, $pid) {
  module_load_include('inc', 'islandora_book', 'management/IslandoraBookPageTable');
  $table = islandora_book_page_table($pid);

  if (user_access('delete pages')) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    $form['delete_page'] = array(
      '#title' => "Permanently Delete Pages from this Book",
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['delete_page']['table'] = array(
      'table' => $table,
    );


    $form['delete_page']['confirm']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete Selected Pages'),
      '#id' => 'delete_pages',
    );
  }

  return $form;
}

/**
 * retrieves all files from temporary directory, including zipped files, and ordering by mtime
 * @param string $pid
 * @return array
 */
function get_page_files($pid) {
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  $flat_pid = str_replace(':', '_', $pid);
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $file_dir = file_directory_path() . "/$flat_pid/";
  $files = scandir($file_dir);

//clean and order array input array

  $temp_process = array();
  foreach ($files as $file) {
    if ($file != '.' && $file != '..') {
      $filename = $file_dir . $file;
      $temp_process[$filename] = filemtime($filename);
    }
  }
  asort($temp_process);
  $sorted_files = array_keys($temp_process);
// unzip files and add them to array in the correct order
// spaces are rmoved from file names in the interest of consistency

  $ordered_files = array();
  foreach ($sorted_files as $candidate) {
    $old_filename = pathinfo($candidate, PATHINFO_FILENAME);
    $ext = pathinfo($candidate, PATHINFO_EXTENSION);
    $new_file_name = preg_replace("/[^a-z\.\d]/i", "", $old_filename);
    if ($new_file_name != $old_filename) {
      $new_fullname = str_replace($old_filename, $new_file_name, $candidate);
      $success = rename("$candidate", "$new_fullname");
      $candidate = $new_fullname;
    }
    if ($ext == 'tif' || $ext == 'tiff') {
      $ordered_files[] = $candidate;
    }
    if ($ext == 'zip') {
      $unzipped_files = get_files_from_zip($candidate);
      $ordered_files = array_merge($ordered_files, $unzipped_files);
    }
  }
  // ordered files now has all the information we need to do a sucessful page ingest

  return $ordered_files;
}

function page_delete_submit($form, &$form_state) {

  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $book_pid = $form_state['values']['pid'];
  $page_pids = @array_filter($form_state['values']['table']['selections']);
  $count = count($page_pids);
  $batch = array(
    'title' => t("Deleting @page_count pages", array('@page_count' => $count)),
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/management/add_pages.inc',
    'init_message' => t('Preparing data...'),
    'progress_message' => t('@current of @total pages deleted.'),
  );

  foreach ($page_pids as $page_pid) {
    $batch['operations'][] = array('islandora_book_delete_single_page', array($page_pid));
    $batch['operations'][] = array('post_deletion_renumber_pages', array($book_pid));
  }
  batch_set($batch);
  batch_process("fedora/repository/$book_pid");
}

function islandora_book_delete_single_page($page_pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  global $user;
  $item = new Fedora_Item($page_pid);
  $item->purge($pid . ' deleted by user identified as ' . $user->name);
}