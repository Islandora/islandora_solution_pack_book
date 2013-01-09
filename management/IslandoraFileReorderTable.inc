<?php

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $collection_pid parameter
 * @param string $collection_pid
 * @return array
 */
function islandora_file_reorder_table($book_pid) {
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

  $files = get_page_files($book_pid);
  if (count($files) < 1) {
    return;
  }
  $file_info = array();
  foreach ($files as $file) {
    $path_parts = pathinfo($file);
    $file_info[pathinfo($file, PATHINFO_FILENAME)] = $file;
  }



  $table = array(
    '#header' => array("", t("File")),
    '#theme' => 'islandora_file_reorder_table',
    '#tree' => TRUE,
    '#id' => 'file_weight_table',
    'rows' => array(),
  );
  $rows = &$table['rows'];
  if (empty($files)) {
    return;
  }
  $pos = 0;
  foreach ($file_info as $key => $object) {
    $cells = array();

    $cells['pos'] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlenth' => 4,
      '#attributes' => array('class' => 'sequence_weight'),
      '#default_value' => $count++
    );

    $cells['label'] = array('#type' => 'item', '#value' => $key);
    $cells['file'] = array('#type' => 'hidden', '#value' => $object);
    $rows[] = $cells;
  }

  return array('table' => $table, 'filecount' => count($files));
}

/**
 * themes the form table.
 *
 * @param array $element Drupal Form Element.
 * @return string
 */
function theme_islandora_file_reorder_table(array $element) {
  $rows = array();
  foreach (element_children($element['rows']) as $child) {
    $setting = $element['rows'][$child];
    $pid = $setting['#pid'];
    $fields = array(
      drupal_render($element['selections'][$pid]) // First field is a checkbox
    );
    foreach (element_children($setting) as $property) {
      $cell_array = array();
      $field = $setting[$property];
      $cell_array['data'] = drupal_render($field);
      if ($property == 'pos') {
        $cell_array['class'] = 'sequence_weightz';
      }
      if ($property == 'pid') {
        $cell_array['class'] = 'fedora_link';
      }
      $fields[] = $cell_array;
    }
    $rows[] = array(
      'data' => $fields,
      'class' => 'draggable',
    );
  }

  drupal_add_tabledrag('file_weight_table', 'order', 'sibling', 'sequence_weight');
  return theme('table', $element['#header'], $rows, array('id' => 'file_weight_table'));
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
  if (!is_dir($file_dir)) {
    return;
  }
  $files = scandir($file_dir);

//clean and order array input array

  $temp_process = array();
  foreach ($files as $file) {
    if ($file != '.' && $file != '..' &! is_dir($file)) {
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
    $new_file_name = preg_replace("/[^a-z\.\d]/i", "_", $old_filename);
    if ($new_file_name != $old_filename) {
      $new_fullname = str_replace($old_filename, $new_file_name, $candidate);
      $success = rename("$candidate", "$new_fullname");
      $candidate = $new_fullname;
    }
    $allowed_extentions = array('tif', 'tiff', 'jpg', 'jpeg');
    if (in_array(strtolower($ext), $allowed_extentions)) {
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