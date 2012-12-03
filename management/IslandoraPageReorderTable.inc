<?php

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $collection_pid parameter
 * @param string $collection_pid
 * @return array
 */
function islandora_book_page_reorder_table($book_pid) {
  global $base_url;
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $query = <<<XML
  select \$object \$page \$title from <#ri>
where \$object <fedora-rels-ext:isMemberOf> <info:fedora/$book_pid>
and \$object <fedora-model:state> <fedora-model:Active>
and \$object <dc:title> \$title
and (\$object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> \$page
or \$object <http://islandora.ca/ontology/relsext#isSequenceNumber> \$page)
order by \$page
XML;
  $query = htmlentities(urlencode($query));
  $content = '';

  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=csv&limit=$limit&offset=$offset&lang=itql&stream=on&query=" . $query;
  $content .= do_curl($url);
  $results = explode("\n", $content);
  $lines = preg_replace('/info:fedora\/|"object","page","title"/', '', $results);
  $lines = array_values(array_filter($lines));
  $keys = array();
  $object = array();
  foreach ($lines as $line) {
    $line_parts = explode(',', $line);

    $objects[$line_parts[0]] = array($line_parts[1], $line_parts[2]);
    $keys[] = $line_parts[0];
  }



  $table = array(
    '#header' => array(theme('table_select_header_cell'), t("Page"), 'Current<br />Sequence #', t('Label')),
    '#theme' => 'islandora_book_page_reorder_table',
    '#tree' => TRUE,
    '#id' => 'page_weight_table',
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => array_fill_keys($keys, ''),
    ),
  );
  $rows = &$table['rows'];
  if (empty($objects)) {
    return;
  }
  foreach ($objects as $key => $object) {
    $alink_options = array(
      'attributes' => array('target' => '_blank', 'class' => 'fedora_image', 'title' => $key),
      'html' => TRUE,
    );
    $img = "<img src='$base_url/fedora/repository/$key/TN' width='28' height='42'>";
    $img = theme_image("$base_url/fedora/repository/$key/TN", '', NULL, array('width' => '20'), FALSE);
    $cells = array();
    $cells['#pid'] = $key;
    $cells['pid'] = array('#value' => l($img, 'fedora/repository/' . $key , $alink_options));

    $cells['sequence'] = array(
      '#type' => 'item',
      '#value' => $object[0],
    );

    $cells['pos'] = array(
      '#type' => 'textfield',
      '#default_value' => $object[0],
      '#size' => 3,
      '#maxlenth' => 4,
      '#attributes' => array('class' => 'sequence_weight'),
    );

    $cells['label'] = array('#type' => 'textfield', '#default_value' => $object[1]);
    $cells['page_pid'] = array('#type' => 'hidden', '#value' => $key);
    $cells['current_sequence'] = array('#type' => 'hidden', '#value' => $object[0]);
    $cells['current_label'] = array('#type' => 'hidden', '#value' => $object[1]);
    $rows[] = $cells;
  }

  return $table;
}

/**
 * themes the form table.
 *
 * @param array $element Drupal Form Element.
 * @return string
 */
function theme_islandora_book_page_reorder_table(array $element) {
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
        $cell_array['class'] = 'sequence_weight';
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

  drupal_add_tabledrag('page_weight_table', 'order', 'sibling', 'sequence_weight');
  return theme('table', $element['#header'], $rows, array('id' => 'page_weight_table'));
}