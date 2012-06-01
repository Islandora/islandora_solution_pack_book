<?php

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $collection_pid parameter
 * @param string $collection_pid
 * @return array
 */


function islandora_book_page_table($book_pid) {
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

  $form['list'] = array(
    '#value' => $list,
  );

  $table = array(
    '#header' => array(theme('table_select_header_cell'), t('Page Pid'), 'Sequence Number', t('Page Label')),
    '#theme' => 'islandora_book_page_table',
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


function theme_islandora_book_page_table(array $element) {
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