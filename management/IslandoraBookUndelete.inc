<?php

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $collection_pid parameter
 * @param string $collection_pid
 * @return array
 */


function islandora_book_undelete_table() {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');

  $lines = islandora_book_get_inactive_books();
  $lines = array_values(array_filter($lines));
  $keys = array();
  $object = array();
  foreach ($lines as $line) {
    $line_parts = explode(',', $line);
    $objects[$line_parts[0]] = array($line_parts[1], $line_parts[2]);
    $keys[] = $line_parts[0];
  }


  $table = array(
    '#header' => array(theme('table_select_header_cell'), t('Book Pid'),  t('Book Title')),
    '#theme' => 'islandora_book_undelete_table',
    '#tree' => TRUE,
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => array_fill_keys($keys, ''),
    ),
  );
  $rows = &$table['rows'];
  if(empty($objects)){
    return;
  }
  foreach ($objects as $key => $object) {
    $rows[] = array(
      '#pid' => $key,
      'pid' => array('#value' => l($key, 'fedora/repository/' . $key)),
      'sequence' => array('#value' => $object[0]),
      'label' => array('#value' => $object[1]),
    );
  }

  return $table;
}



/**
 * themes the form table.
 *
 * @param array $element Drupal Form Element.
 * @return string
 */


function theme_islandora_book_undelete_table(array $element) {
  $rows = array();
  foreach (element_children($element['rows']) as $child) {
    $setting = $element['rows'][$child];
    $pid = $setting['#pid'];
    $fields = array(
      drupal_render($element['selections'][$pid]) // First field is a checkbox
    );
    foreach (element_children($setting) as $property) {
      $field = $setting[$property];
      $fields[] = drupal_render($field);
    }
    $rows[] = array(
      'data' => $fields,
      'class' => isset($setting['#attributes']['class']) ? $setting['#attributes']['class'] : NULL
    );
  }
  $attributes = isset($element['#id']) ? array('id' => $element['#id']) : NULL;
  return theme_table($element['#header'], $rows, $attributes);
}