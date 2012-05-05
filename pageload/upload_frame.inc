<?php

function upload_pages($pid) {
  global $base_url;
  $path = drupal_get_path('module', 'islandora_book');
  $url = "$base_url/$path/pageload/loader.php?PID=$pid";
  $html = '<iframe src="' . $url . '" scrolling="0" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';
  return $html;
}

function upload_setup($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $label = $item->objectProfile->objLabel;
  global $base_url;
  global $base_path;
  unset($_SESSION['script_url']);
  $flat_pid = str_replace(':', '_', $pid);
  $upload_dir = $_SERVER['DOCUMENT_ROOT'] . $base_path . file_directory_path() . "/$flat_pid/";
  $upload_dir = str_replace('//', '/', $upload_dir);
  $save_path = $base_url . '/' . file_directory_path() . "/$flat_pid/";
  $tmpDir = file_directory_path() . '/' . $flat_pid . '/';
  if (!is_dir($tmpDir)) {
    mkdir($tmpDir, 0766);
  }
  $thumbnail_dir = "$tmpDir/thumbnails";
  if (!is_dir($thumbnail_dir)) {
    mkdir($thumbnail_dir, 0766);
  }
  $_SESSION['script_url'] = $base_url . '/pageview/uploadHandler.inc';

  $results = array();
  $results['dir_callback'] = $base_url . "/islandora/book/process_page_directory/$pid";
  $results['book_callback'] = $base_url . "/islandora/book/process_file/$flat_pid";
  $results['delete_callback'] = $base_url . "/islandora/book/delete_book_files/$flat_pid";
  $results['book_title'] = $label;

  $json = json_encode($results);
  echo $json;
}

function process_file($flat_pid) {

  error_reporting(E_ALL | E_STRICT);
  module_load_include('inc', 'islandora_book', 'pageload/uploadHandler');
  // require('upload.class.php');

  $upload_handler = new UploadHandler($flat_pid);

  header('Pragma: no-cache');
  header('Cache-Control: no-store, no-cache, must-revalidate');
  header('Content-Disposition: inline; filename="files.json"');
  header('X-Content-Type-Options: nosniff');
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: OPTIONS, HEAD, GET, POST, PUT, DELETE');
  header('Access-Control-Allow-Headers: X-File-Name, X-File-Type, X-File-Size');

  switch ($_SERVER['REQUEST_METHOD']) {
    case 'OPTIONS':
      break;
    case 'HEAD':
    case 'GET':
      $upload_handler->get();
      break;
    case 'POST':
      if (isset($_REQUEST['_method']) && $_REQUEST['_method'] === 'DELETE') {
        $upload_handler->delete();
      }
      else {
        $upload_handler->post();
      }
      break;
    case 'DELETE':
      $upload_handler->delete();
      break;
    default:
      header('HTTP/1.1 405 Method Not Allowed');
  }
}

function delete_book_page_files($pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $dir = file_directory_path() . "/$pid";
  islandora_recursive_directory_delete($dir);
 
}