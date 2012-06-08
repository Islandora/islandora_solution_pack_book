<?php

/**
 * retrieves all files from temporary directory, including zipped files, and ordering by mtime
 * @param string $pid
 * @return array
 */
function prep_add_form($pid) {
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  $content = drupal_get_form('book_add_pages_form', $pid);

  return $content;
}

