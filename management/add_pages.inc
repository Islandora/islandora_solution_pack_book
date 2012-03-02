<?php

/**
 * book add pages form
 * @param array $form_state
 * @param string $pid
 * @param string $page_cmodel
 * @param string $relation
 * @return array
 */
function book_add_pages(&$form_state, $pid, $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOf') {
  $file = (isset($form_state['values']['ingest-file-location']) ? $form_state['values']['ingest-file-location'] : '');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
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
    '#collapsible' => true,
    '#title' => t('Add pages to this book'),
    '#description' => t('Add pages to this book object.'),
  );
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
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

  $form['page_add']['file_upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload Zipped TIFFs'),
    '#size' => 48,
    '#description' => t('Upload a zip file containing a tiff for each page of this book'),
  );
  $form['page_add']['upload_from_server'] = array(
    '#type' => 'textfield',
    '#description' => 'Server location of previously uploaded file',
    '#title' => 'FTP file location',
  );

  $form['page_add']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 10,
    '#value' => 'Ingest Pages'
  );
  return $form;
}

/**
 * book add pages form validate
 * @param array $form
 * @param array $form_state
 */
function book_add_pages_validate($form, &$form_state) {
  $_SESSION['manage_chosen'] = TRUE;
  if (isset($form_state['values']['insertion_point'])) {
    $insertion_point = $form_state['values']['insertion_point'] + 0;
    $count = $form_state['values']['page_count'] + 0;
    if (!is_int($insertion_point) || $insertion_point > $count || $insertion_point < 0) {
      form_set_error('insertion_point', "Number must be a positive integer less than or equal to $count");
      return;
    }
  }
  $fieldName = 'file_upload';
  if (isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])) {
    $file = file_save_upload($fieldName);
    if ($file->filemime != 'application/zip') {
      form_set_error($fieldName, 'Input file must be a .zip file');
      return;
    }
    if (!$file) {
      form_set_error($fieldName, 'Error uploading file.');
      return;
    }
    $form_state['values']['file'] = $file->filepath;
  }

  if ($form_state['values']['upload_from_server']) {
    if (is_file($form_state['values']['upload_from_server'])) {
      $form_state['values']['file'] = $form_state['values']['upload_from_server'];
    }
    else {
      form_set_error('upload_from_server', 'File not found.');
      return;
    }
  }

  if (!isset($form_state['values']['file'])) {
    form_set_error($fieldName, 'Exactly one of these fields must be populated.');
    return;
  }
  unset($_SESSION['manage_chosen']);
}

/**
 * book add pages form submit
 * @param array $form
 * @param array $form_state
 */
function book_add_pages_submit($form, &$form_state) {
  $pid = $form_state['values']['pid'];
  $start_page = (isset($form_state['values']['insertion_point'])) ? $form_state['values']['insertion_point'] + 1 : 1;
  $dirName = str_replace(':', '_', $pid);
  $current_files = scandir(file_directory_path());
  foreach ($current_files as $current_file) {
    if ($current_file != '.' && $current_file != '..' && is_dir($current_file)) {
      $fileage = time() - filectime(file_directory_path() . "/$current_file");
      if ($fileage > 60 * 60) {
        deleteDirectory(file_directory_path() . "/$current_file");
      }
    }
  }
  $tmpDir = file_directory_path() . '/' . $dirName . '/';
  if (!is_dir($tmpDir)) {
    mkdir($tmpDir, 0766);
  }
  $zip_file = $form_state['values']['file'];

  // $zip_file = $file->filepath;
  $file_list = array();
  $cmdString = "unzip -q -o -d $tmpDir \"$zip_file\"";

  system($cmdString, $retVal);
  file_delete("$tmpDir \"$zip_file\"");

  $dirs = array();
  array_push($dirs, $tmpDir);
  $files = scandir($tmpDir);

  foreach ($files as $file) {
    if (is_dir("$tmpDir/$file") && $file != '.' && $file != '..' && $file != '__MACOSX') {
      $new_file_name = preg_replace("/[^a-z\.\d]/i", "", $file);
      if ($new_file_name != $file) {
        rename("$tmpDir/$file", "$tmpDir/$new_file_name");
        $file = $new_file_name;
      }
      array_push($dirs, $tmpDir . $file);
    }
  }

  foreach ($dirs as $directory) {
    if ($tiff_dir = opendir($directory)) {
      while (FALSE !== ($file_name = readdir($tiff_dir))) {
        $fullfile = $directory . $file_name;
        if (!is_dir($fullfile)) {
          $new_file_name = preg_replace("/[^a-z\.\d]/i", "", $file_name);
          $new_file_name = preg_replace("/\.tiff$/", ".tif", $new_file_name);
          if ($new_file_name != $file_name) {
            rename($directory . "/" . $file_name, $directory . "/$new_file_name");
            $file_name = $new_file_name;
          }
        }
        $ext = strrchr($file_name, '.');
        if ($ext == '.tif' || $ext == '.tiff') {
          array_push($file_list, "$directory" . "/$file_name");
        }
      }
      closedir($tiff_dir);
    }
  }
  sort($file_list);
  ibook_create_page_objects($pid, $file_list, $form_state['values']['page_cmodel'], $form_state['values']['relation'], $tmpDir, $start_page);
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
  $batch['operations'][] = array('makePDF', array($pid, $image_list, $tmpDir));
  batch_set($batch);
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
  deleteDirectory($tmpDir);
}

function page_delete(&$form_state, $pid) {

  if (user_access('delete pages')) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    $form['delete_page'] = array(
      '#title' => "Permanently Delete Pages from this Book",
      '#type' => 'fieldset',
      '#description' => t("Clicking this button will delete this all book pages. <br /> <strong>This action cannot be undone.</strong>"),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['delete_page']['confirm'] = array(
      '#title' => "Are you sure?",
      '#type' => 'fieldset',
      '#description' => t('<strong>Clicking the delete button will permanantly remove all of its pages.</strong> <br />  <strong>This action cannot be undone.</strong> '),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['delete_page']['confirm']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete All Pages'),
      '#id' => 'delete_pages',
    );
  }

  return $form;
}

function page_delete_submit($form, &$form_state) {
  $book_pid = $form_state['values']['pid'];
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $page_pids = get_page_pids($book_pid);
  foreach ($page_pids as $page_pid) {
    $item = new Fedora_Item($page_pid);
    $item->purge();
  }
}

function renumber_pages($insertion_point, $book_pid, $new_page_count) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $page_pids = get_sorted_pages($book_pid);
  for ($i = $insertion_point; $i <= count($page_pids); ++$i) {
    $item = new Fedora_Item($page_pids[$i]);
    if ($item->exists()) {
      $item->purge_relationship('isPageNumber', sprintf("%04d", $i));
      $item->add_relationship('isPageNumber', sprintf("%04d", $i + $new_page_count), ISLANDORA_PAGE_URI);
    }
  }
}