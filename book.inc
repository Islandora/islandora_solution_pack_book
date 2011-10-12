<?php

/**
 * @file 
 * Islandora book 
 */

/**
 * Islandora Book
 */
class IslandoraBook {

  /**
   * Constructor
   * @param type $pid 
   */
  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  /**
   * Show field sets
   * @global type $base_url
   * @global type $user
   * @return string 
   */
  public function showFieldSets() {
    module_load_include('inc', 'fedora_repository', 'plugins/tagging_form');
    module_load_include('inc', 'fedora_repository', 'plugins/DarwinCore');
    module_load_include('module', 'islandora_book');
    global $base_url;
    $show_purge_tab = (!empty($_POST['form_id']) && ($_POST['form_id'] == 'islandora_book_purge_pages_form'));
    $first_page = new Fedora_Item($this->pid . '-001');
    $hasPage = $first_page->exists();
    $tabset = array();

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }

    $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/islandora:viewerSdef/getViewer' . $qs;
    $html = '<iframe src="' . $viewer_url . '" scrolling="0" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';

    $tabset['first_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Description'),
      // This will be the content of the tab.
      '#content' => islandora_book_create_book_view($this->pid),
    );

    if ($hasPage) {
      $tabset['second_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Read'),
        '#content' => $html
      );
    }

    if (!$hasPage && user_access('ingest new fedora objects')) {
      $tabset['third_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add pages'),
        '#content' => drupal_get_form('book_add_pages_form', $this->pid),
      );
    }
    if ($hasPage && user_access('purge objects and datastreams')) {
      $tabset['fourth_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Purge all pages'),
        '#selected' => $show_purge_tab,
        '#content' => drupal_get_form('islandora_book_purge_pages_form', $this->pid),
      );
    }
    return $tabset;
  }

}

/**
 * book add pages form
 * @param type $form_state
 * @param type $pid
 * @param type $page_cmodel
 * @param type $relation
 * @return int 
 */
function book_add_pages_form(&$form_state, $pid, $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOf') {
  $file = (isset($form_state['values']['ingest-file-location']) ? $form_state['values']['ingest-file-location'] : '');
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['page_cmodel'] = array(
    '#type' => 'hidden',
    '#value' => $page_cmodel,
  );
  $form['relation'] = array(
    '#type' => 'hidden',
    '#value' => $relation,
  );

  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['file_upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload Zipped TIFFs'),
    '#size' => 48,
    '#description' => t('Upload a zip file containing a tiff for each page of this book'), );

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 10,
    '#value' => 'Ingest Pages'
  );
  return $form;
}

/**
 * book add pages form validate
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function book_add_pages_form_validate($form, &$form_state) {

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
    $form_state['values']['file'] = $file;
  }
  else {
    // set error
    form_set_error($fieldName, 'Error uploading file.');
    return;
  }
}

/**
 * book add pages form submit
 * @param type $form
 * @param type $form_state 
 */
function book_add_pages_form_submit($form, &$form_state) {
  $pid = $form_state['values']['pid'];
  $dirName = str_replace(':', '_', $pid);
  $tmpDir = file_directory_path() . '/' . $dirName . '/';
  mkdir($tmpDir);
  $file = $form_state['values']['file'];

  $fileName = $file->filepath;
  $file_list = array();
  $cmdString = "unzip -q -o -d $tmpDir \"$fileName\"";

  system($cmdString, $retVal);

  $dirs = array();
  array_push($dirs, $tmpDir);
  $files = scandir($tmpDir);
  foreach ($files as $file) {
    if (is_dir("$tmpDir/$file") && $file != '.' && $file != '..' && $file != '__MACOSX') {
      array_push($dirs, $tmpDir . $file);
    }
  }

  foreach ($dirs as $directory) {
    if ($tiff_dir = opendir($directory)) {
      while (FALSE !== ($file_name = readdir($tiff_dir))) {
        $ext = strrchr($file_name, '.');
        if ($ext == '.tif' || $ext == '.tiff') {
          array_push($file_list, "$directory/$file_name");
        }
      }
      closedir($tiff_dir);
    }
  }
  sort($file_list);
  ibook_create_page_objects($pid, $file_list, $form_state['values']['page_cmodel'], $form_state['values']['relation'], $tmpDir);
}

/**
 * Create a page object for each element in $image_list as a child object of $pid
 * @param type $pid
 * @param type $image_list
 * @param type $page_cmodel
 * @param type $relation
 * @param type $tmpDir
 * @return type 
 */
function ibook_create_page_objects($pid, $image_list = array(), $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOfCollection', $tmpDir) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $book = new Fedora_Item($pid);
  if (!$book->exists()) {
    return FALSE;
  }
  $dirName = str_replace(':', '_', $pid);

  $page_pids = array();
  $batch = array(
    'title' => 'Creating page objects',
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/book.inc',
  );
  $page_num = 1;
  $book_title = $book->objectProfile->objLabel;
  foreach ($image_list as $image_path) {
    $batch['operations'][] = array('ibook_add_single_page_object', array($pid, $image_path, $page_num, "$book_title - Page " . sprintf("%03d", $page_num), $page_cmodel, $relation));
    $page_num++;
  }
  $batch['operations'][] = array('makePDF', array($pid, $image_list, $tmpDir));
  $batch['operations'][] = array('deleteDirectory', array($tmpDir));

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
    $book->modify_datastream_by_value($xml->saveXML(), 'DC', 'Dublin core', 'text/xml', TRUE, "Added Type of Ingest", FALSE);
  }
}

/**
 * ibook add single page object
 * @param type $book_pid
 * @param type $page_file
 * @param type $page_num
 * @param type $page_title
 * @param type $page_cmodel
 * @param type $relation 
 */
function ibook_add_single_page_object($book_pid, $page_file, $page_num = 1, $page_title = NULL, $page_cmodel = 'islandora:pageCModel', $relation = 'isMemberOf') {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $page_item = Fedora_Item::ingest_new_item("$book_pid-" . sprintf("%03d", $page_num), 'A', $page_title);
  $page_item->add_relationship('hasModel', $page_cmodel, FEDORA_MODEL_URI);
  $page_item->add_relationship($relation, $book_pid);
  // add original tiff
  $page_item->add_datastream_from_file($page_file, 'TIFF', 'Archival TIFF', 'image/tiff', 'M');

  //add compresed jp2 for viewer
  if (variable_get('islandora_book_create_images', TRUE)) {
    exec('kdu_compress -i "' . $page_file . '" -o "' . $page_file . '.jp2' . '" -rate 0.5 Clayers=1 Clevels=7 "Cprecincts={256,256},{256,256},{256,256},{128,128},{128,128},{64,64},{64,64},{32,32},{16,16}" "Corder=RPCL" "ORGgen_plt=yes" "ORGtparts=R" "Cblk={32,32}" Cuse_sop=yes', $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file.jp2", 'JP2', "Compressed JP2", 'image/jp2', 'M');
    }

    //add thumbnail image
    exec("convert {$page_file}[0] -thumbnail x2000 -thumbnail x450  -resize '450x<' -resize 50% -fuzz 1% -trim +repage -gravity center -crop 200x200+0+0 +repage -format jpg -quality 100  $page_file-tn.jpg", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
      $book_item = new Fedora_Item($book_pid);
      $DSIDS = $book_item->get_datastreams_list_as_array();
      if (!array_key_exists("TN", $DSIDS)) {
        $book_item->add_datastream_from_file("$page_file-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
      }
    }

    //add jpeg
    exec("convert  -resize 800 \"{$page_file}\"[0]  \"$page_file-med.jpg\"", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file-med.jpg", 'JPEG', "JPEG image", 'image/jpeg', 'M');
    }
  }
  //perform OCR using Tesseract, add text datastream

  if (variable_get('islandora_book_do_ocr', TRUE)) {
    $executable = variable_get('islandora_ocr_path', '/usr/bin/tesseract');
    exec("$executable $page_file $page_file -l eng", $output, $returnValue);
    if ($returnValue == '0') {
      $page_item->add_datastream_from_file("$page_file.txt", 'OCR', "Scanned Text", 'text/plain', 'M');
    }
  }
}

/**
 * delete directory
 * @param type $dir
 * @return type 
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
 * @param type $book_pid
 * @param type $file_list
 * @param type $tmpDir
 * @param type tmpDir 
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
  if ($returnValue == '0') {
    $book_item->add_datastream_from_file($outputFile, 'PDF', "PDF of Book", 'application/pdf', 'M');
  }
}

