<?php

/**
 *
 * Returns the pids of all pages associated with a book object
 * @param  string $book_pid
 * @return string
 */
function get_page_pids($book_pid) {
  return get_sorted_pages($book_pid);
}

/**
 *
 * Replaces OCR scan in page object
 *
 * @param string $tiff
 * @param fedora item $item
 * @return boolean
 */
function refresh_ocr($tiff, &$item, $language, $preprocess = FALSE) {
  $label = $item->objectProfile->objLabel;
  $datastreams = $item->get_datastreams_list_as_array();
  $executable = variable_get('islandora_ocr_path', '/usr/bin/tesseract');
  $returnValue = "";
  $output = "";
  $success = FALSE;
  $original = $tiff;


  if ($preprocess) {
    $command = <<<XML
  gimp -d -i -b "(munge-typeset-ocr \"$tiff\" \"$tiff.png\" 5.0 10.0 0.0 200.0 5.0)" -b "(gimp-quit 0)"
XML;
    exec($command, $output, $returnVal);

    if ($returnVal == 0) {
      if (file_exists($tiff . png)) {
        $tiff = "$tiff.png";
      }
    }
  }
  $command = "$executable $tiff $tiff -l $language";
  exec($command, $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('OCR', $datastreams)) {
      $item->purge_datastream('OCR');
    }
    if (array_key_exists('CWRC', $datastreams)) {
      $item->purge_datastream('CWRC');
    }
    $item->add_datastream_from_file("$tiff.txt", 'OCR', "Scanned Text", 'text/plain', 'M');
    file_delete("$tiff.txt");
   
    $success = TRUE;
  }
  if (!$success) {
    drupal_set_message(t('OCR replacement for @pid was unsuccessful', array('@pid' => $item->pid)));
  }
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  $success = islandora_book_viewer_create_hocr($executable, $tiff, $language, $item, $datastreams);
  if (!$success) {
    drupal_set_message(t('ENCODED_OCR replacement for @pid was unsuccessful', array('@pid' => $item->pid)));
  }
   if ($tiff != $original) {
      file_delete($tiff);
    }
  return $success;
}

/**
 *
 * Replaces derivative images in page object
 *
 * @param string $tiff
 * @param fedora item $item
 * @return boolean
 */
function refresh_images($tiff, &$item) {
  $label = $item->objectProfile->objLabel;
  $datastreams = $item->get_datastreams_list_as_array();
  $returnValue = "";
  $success = 0;
  $command = 'kdu_compress -i "' . $tiff . '" -o "' . $tiff . '.jp2' . '" -rate 0.5 Clayers=1 Clevels=7 "Cprecincts={256,256},{256,256},{256,256},{128,128},{128,128},{64,64},{64,64},{32,32},{16,16}" "Corder=RPCL" "ORGgen_plt=yes" "ORGtparts=R" "Cblk={32,32}" Cuse_sop=yes';

  exec($command, $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('JP2', $datastreams)) {
      $item->purge_datastream('JP2');
    }
    $item->add_datastream_from_file("$tiff.jp2", 'JP2', "Compressed JP2", 'image/jp2', 'M');
    file_delete("$tiff.jp2");
    $success++;
  }

  exec("convert {$tiff}[0] -colorspace RGB -thumbnail 120x $tiff-tn.jpg", $output, $returnValue);

  if ($returnValue == '0') {
    if (array_key_exists('TN', $datastreams)) {
      $item->purge_datastream('TN');
    }
    $item->add_datastream_from_file("$tiff-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
    file_delete("$tiff-tn.jpg");
    $success++;
  }
  file_delete("$tiff-tn.jpg");
  exec("convert  -resize 800 \"{$tiff}\"[0]  \"$tiff-med.jpg\"", $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('JPEG', $datastreams)) {
      $item->purge_datastream('JPEG');
    }
    $item->add_datastream_from_file("$tiff-med.jpg", 'JPEG', "JPEG image", 'image/jpeg', 'M');
    file_delete("$tiff-med.jpg");
    $success++;
  }
  return $success;
}

/**
 *
 * Copies file identified by datastream ID to tempfile
 * and returns full directory path
 *
 * @param string $pid
 * @param string $dsid
 * @return string
 */
function save_datastream_as_tempfile($pid, $dsid, $ext = '.tif') {
  $tmp_filename = preg_replace("/:|-/", '_', $pid) . $ext;
  $item = new Fedora_Item($pid);
  if (array_key_exists('TIFF', $item->datastreams)) {
    $dsid = 'TIFF';
  }
  if (array_key_exists('TIF', $item->datastreams)) {
    $dsid = 'TIF';
  }
  $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "/objects/$pid/datastreams/$dsid/content";
  $file_contents = do_curl($url);
  $file_path = file_directory_path() . "/" . $tmp_filename;
  file_put_contents($file_path, $file_contents);
  return $file_path;
}

/**
 *
 * Moves tiffs for all pages associated with a book to a tempdir
 * and returns the full directory path
 *
 * @param string $pid
 * @param string $dsid
 * @return string
 */
function save_all_tiffs_in_tempdir($pid, $dsid) {
  $tmp_dirname = file_directory_path() . "/" . preg_replace("/:|-/", '_', $pid);
  mkdir($tmp_dirname);
  $member_pids = get_page_pids($pid);
  foreach ($member_pids as $member_pid) {
    $item = new Fedora_Item($member_pid);
    if (array_key_exists('TIFF', $item->datastreams)) {
      $dsid = 'TIFF';
    }
    if (array_key_exists('TIF', $item->datastreams)) {
      $dsid = 'TIF';
    }
    $tmp_filename = preg_replace("/:|-/", '_', $member_pid) . '.tif';
    $url = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "/objects/$member_pid/datastreams/$dsid/content";
    $file_contents = do_curl($url);
    $file_path = $tmp_dirname . "/" . $tmp_filename;
    file_put_contents($file_path, $file_contents);
  }

  return $tmp_dirname;
}

/**
 *
 * Returns tabular display of current datastreams
 *
 * @global string $fedoraUser
 * @global string $fedoraPass
 * @global url $base_url
 * @global user $user
 * @param  string $object_pid
 * @param Fedora_Item $fedoraItem
 * @return string
 */
function datastream_display_builder($object_pid, &$fedoraItem) {
  global $fedoraUser, $fedoraPass, $base_url, $user;
  module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'ContentModel');
  $object_helper = new ObjectHelper();
  $path = drupal_get_path('module', 'fedora_repository');
  $dataStreamBody = '';
  $fedoraItem = new Fedora_Item($object_pid);

  if (user_access(ObjectHelper :: $VIEW_DETAILED_CONTENT_LIST)) {
    $current_datastreams = 'All Datastreams';
//$metaDataText='Description';
    $mainStreamLabel = NULL;
    $object = $fedoraItem->get_datastreams_list_as_SimpleXML();
    if (!isset($object)) {
      drupal_set_message(t("No datastreams available"));
      return ' ';
    }
    $hasOBJStream = NULL;
    $hasTNStream = FALSE;
    $dataStreamBody = "<br /><table>\n";

    $cmDatastreams = array();
    if (variable_get('fedora_object_restrict_datastreams', FALSE) == TRUE && ($cm = ContentModel::loadFromObject($object_pid)) !== FALSE) {
      $cmDatastreams = $cm->listDatastreams();
    }

    $dataStreamBody .= $object_helper->get_parent_objects_asHTML($object_pid);
    $dataStreamBody .= '<tr><th colspan="4"><h3>' . t("!text", array('!text' => $current_datastreams)) . '</h3></th></tr>';
    foreach ($object as $datastream) {
      foreach ($datastream as $datastreamValue) {
        if (variable_get('fedora_object_restrict_datastreams', FALSE) == FALSE || ((isset($user) && in_array('administrator', $user->roles)) || in_array($datastreamValue->ID, $cmDatastreams))) {
          if ($datastreamValue->ID == 'OBJ') {
            $hasOBJStream = '1';
            $mainStreamLabel = $datastreamValue->label;
            $mainStreamLabel = str_replace("_", " ", $mainStreamLabel);
          }
          if ($datastreamValue->ID == 'TN') {
            $hasTNStream = TRUE;
          }
//create the links to each datastream
          $dataStreamBody .= $object_helper->create_link_for_ds($object_pid, $datastreamValue); //"<tr><td><b>$key :</b></td><td>$value</td></tr>\n";
        }
      }
    }
    $dataStreamBody .= "</table>\n";
    return $dataStreamBody;



//if they have access let them add a datastream
    if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($object_pid, 'write');
      }
      if ($allow) {
        $dataStreamBody .= drupal_get_form('add_stream_form', $object_pid);
      }
    }
    $fieldset = array(
      '#title' => t("!text", array('!text' => $availableDataStreamsText)),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#value' => $dataStreamBody
    );
    $dataStreamBody = '<div class = "fedora_detailed_list">' . theme('fieldset', $fieldset) . '</div>';

    return $dataStreamBody;
  }
  return '';
}

/**
 * returns form allowing the ingestion of additional datastreams
 *
 * @param string $object_pid
 * @return array
 */
function get_add_datastream_data($object_pid) {
  if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
    $allow = TRUE;
    if (module_exists('fedora_fesl')) {
      $allow = fedora_fesl_check_roles($object_pid, 'write');
    }
    if ($allow) {
      $dataStreamBody = drupal_get_form('add_stream_form', $object_pid);
    }
  }
  return $dataStreamBody;
}

/**
 * Returns an array of collection pids for each collection
 * with which this pid has an isMember or isMemberOfCollection relationship
 * @param string $pid
 * @return array
 */
function get_collection_from_pid($pid) {

  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $query_string = 'select  $parent  from <#ri>
		where ($object <fedora-rels-ext:isMemberOf> $parent
    or $object <fedora-rels-ext:isMemberOfCollection> $parent)
		and $object <dc:identifier> \'' . $pid . '\'
		order by $object';

  $query_string = htmlentities(urlencode($query_string));
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= '?type=tuples&flush=true&format=csv&limit=13000&lang=itql&stream=on&query=' . $query_string;
  $content = do_curl($url, TRUE);
  $results = explode("\n", $content);
  $object_pids = preg_replace('/^info:fedora\/|"parent"| /', '', $results);
  $object_pids = array_values(array_filter($object_pids));
  return $object_pids;
}

/**
 * Returns an associative array of collection pids and labels for each collection
 * with which this pid has an isMember or isMemberOfCollection relationship
 * @param string $pid
 * @return array
 */
function get_collection_relationships($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $current_relationships = array();
  $item = new fedora_item($pid);
  $collections = $item->get_relationships('isMemberOfCollection');
  foreach ($collections as $collection) {
    $collection_item = new fedora_item($collection['object']);
    if (is_string($collection['object'])) {
      $current_relationships[$collection['object']] = $collection_item->objectProfile->objLabel . ' (' . $collection['object'] . ')';
    }
  }

  return $current_relationships;
}

/**
 * delete directory
 * @param string $dir
 * @return boolen
 */
function islandora_recursive_directory_delete($dir) {
  if (!file_exists($dir))
    return TRUE;
  if (!is_dir($dir))
    return unlink($dir);
  foreach (scandir($dir) as $item) {
    if ($item == '.' || $item == '..')
      continue;
    if (!islandora_recursive_directory_delete($dir . DIRECTORY_SEPARATOR . $item))
      return FALSE;
  }
  return rmdir($dir);
}

function get_sorted_pages($book_pid) {
  $query = <<<XML
  select \$object \$page from <#ri>
where \$object <fedora-rels-ext:isMemberOf> <info:fedora/$book_pid>
and \$object <fedora-model:state> <fedora-model:Active>
and (\$object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> \$page
or \$object <http://islandora.ca/ontology/relsext#isSequenceNumber> \$page)
order by \$page
XML;

  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $collection = new CollectionClass($book_pid);
  $contents = $collection->getRelatedItems($book_pid, $query);
  $results_xml = simplexml_load_string($contents);
  $resultsarray = array();
  $book_pids = array();
  foreach ($results_xml->results->result as $result) {
    $page = str_replace('info:fedora/', '', $result->object->attributes()->uri);
    $number = (string) $result->page[0];
    $book_pids[$number] = $page;
  }

  return $book_pids;
}

function get_page_progression($book_pid) {
  $query = <<<XML
select \$pageProgression from <#ri> where <info:fedora/$book_pid> <fedora-rels-ext:hasPageProgression> \$pageProgression
XML;

  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $collection = new CollectionClass($book_pid);
  $contents = $collection->getRelatedItems($book_pid, $query);
  $results_xml = simplexml_load_string($contents);
  $res = $results_xml->results->result[0];
  if (!empty($res)) {
    return str_replace('info:fedora/', '', $res->pageProgression->attributes()->uri);
  }
  return "lr";
}

function add_islandora_rdfs($book_pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $page_pids = get_page_pids($book_pid);
  $page_num = 1;
  foreach ($page_pids as $pid) {
    $item = new Fedora_Item($pid);
    $relationships = $item->get_rdf_relationships();
    if (!array_key_exists('isPageNumber', $relationships)) {
      $item->add_relationship('isSequenceNumber', $page_num++, 'http://islandora.ca/ontology/relsext#');
    }
    if (!array_key_exists('isPageOf', $relationships)) {
      $item->add_relationship('isPageOf', $book_pid, 'http://islandora.ca/ontology/relsext#');
    }
    //$page_num++;
  }
}

/**
 * Retrieves a sorted array of all files within a nested zip file.
 * @param file $zipped_file
 * @return array
 */
function get_files_from_zip($zip_file) {
  $file_dir = pathinfo($zip_file, PATHINFO_DIRNAME);
  $tmpDir = $file_dir . '/tmpZip/';
  if (!is_dir($tmpDir)) {
    mkdir($tmpDir, 0766);
  }
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

        $ext = pathinfo($file_name, PATHINFO_EXTENSION);
        $allowed_extentions = array('tif', 'tiff', 'jpg', 'jpeg');
        if (in_array(strtolower($ext), $allowed_extentions)) {
          array_push($file_list, "$directory" . "/$file_name");
        }
      }
      closedir($tiff_dir);
    }
  }
  sort($file_list);
  return $file_list;
}

/**
 * Renumbers all pages associated with a book
 *
 * @param integer $insertion_point
 * @param string $book_pid
 * @param integer $new_page_count
 */
function renumber_pages($insertion_point, $book_pid, $new_page_count) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $page_pids = get_sorted_pages($book_pid);


  for ($i = $insertion_point; $i <= count($page_pids); ++$i) {
    $pid = $page_pids[$i];
    $sequence = $i + 1;
    $item = new Fedora_Item($pid);
    if ($item->exists()) {
      $rdf_string = islandora_book_create_rdf($pid, $book_pid, $sequence);
      $value = $item->modify_datastream_by_value($rdf_string, 'RELS-EXT', 'RELS-EXT', 'text/xml', $force = FALSE, $logMessage = 'Modified by Drush using Islandora API', $quiet = FALSE);

      $item->forget();
    }
  }
}

/**
 * Removes gaps from sequencing
 * @param pid $book_pid
 */
function post_deletion_renumber_pages($book_pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $page_pids = get_sorted_pages($book_pid);
  $page_pids = array_values($page_pids);
  $book_item = new Fedora_Item($book_pid);
  $book_title = $book_item->objectProfile->objLabel;

  for ($i = 0; $i < count($page_pids); ++$i) {
    $pid = $page_pids[$i];
    $item = new Fedora_Item($pid);
    if ($item->exists()) {
      $current_number = get_page_number($page_pids[$i]);
      $sequence = $i + 1;
      if ($current_number != $i + 1) {
        $rdf_string = islandora_book_create_rdf($pid, $book_pid, $sequence);
        $value = $item->modify_datastream_by_value($rdf_string, 'RELS-EXT', 'RELS-EXT', 'text/xml', $force = FALSE, $logMessage = 'Modified by Drush using Islandora API', $quiet = FALSE);
      }
    }
  }
}

/**
 * Returns page number associated wih page pid
 * @param string $page_pid
 * @return int>
 */
function get_page_number($page_pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $query_string = "select \$object from <#ri>
                   where <info:fedora/$page_pid> <http://islandora.ca/ontology/relsext#isSequenceNumber> \$object";
  $query_string = htmlentities(urlencode($query_string));
  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= '?type=tuples&flush=true&format=csv&limit=13000&lang=itql&stream=on&query=' . $query_string;
  $content .= do_curl($url);
  $results = explode("\n", $content);
  $pid = preg_replace('/^info:fedora\/|"object"| /', '', $results);
  $pid = array_values(array_filter($pid));
  return intval($pid[0]);
}

/**
 * Returns RELS-EXT stream
 * @param pid> $pid
 * @param pid $book_pid
 * @param int $sequence
 * @return string>
 */
function islandora_book_create_rdf($pid, $book_pid, $sequence) {
  $rdf_string = <<<RDF
  <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
<rdf:Description rdf:about="info:fedora/$pid">
	<hasModel xmlns="info:fedora/fedora-system:def/model#" rdf:resource="info:fedora/islandora:pageCModel"></hasModel>
	<isSequenceNumber xmlns="http://islandora.ca/ontology/relsext#">$sequence</isSequenceNumber>
	<isMemberOf xmlns="info:fedora/fedora-system:def/relations-external#" rdf:resource="info:fedora/$book_pid"></isMemberOf>
	<isSection xmlns="http://islandora.ca/ontology/relsext#">1</isSection>
	<isPageOf xmlns="http://islandora.ca/ontology/relsext#" rdf:resource="inf:fedora/$book_pid"></isPageOf>
  <isPageNumber xmlns="http://islandora.ca/ontology/relsext#">$sequence</isPageNumber>
</rdf:Description>
</rdf:RDF>
RDF;
  return $rdf_string;
}

function islandora_book_get_tesseract_langauges(){
    return array(
    'eng' => t('English'),
    'deu-frak' => t('German'),
    'por' => t('Portugese'),
    'fra' => t('French'),
    'spa' => t('Spanish'),
    'hin' => t('Hindi'),
    'jpn' => t('Japanese'),
    'ita' => t('Italian')
  );

}