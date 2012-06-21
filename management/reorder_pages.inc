<?php

function page_reorder(&$form_state, $pid) {
  module_load_include('inc', 'islandora_book', 'management/IslandoraPageReorderTable');
  $table = islandora_book_page_reorder_table($pid);

  if (user_access('manage book object')) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    $form['reorder_pages'] = array(
      '#title' => t("Reorder pages"),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['reorder_pages']['table'] = array(
      'table' => $table,
    );


    $form['reorder_pages']['confirm']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Reorder Pages'),
    );
  }
  return $form;
}

function page_reorder_submit($form, &$form_state) {

  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $pages = $form_state['values']['table']['rows'];
  foreach ($pages as $page) {
    $new_sequence = ($page['pos'] + 1) / 2;
    if ($new_sequence != $page['current_sequence']) {
      $item = new Fedora_Item($page['page_pid']);
      $item->purge_relationships('isSequenceNumber', $page['current_sequence'], ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
      $item->add_relationship('isSequenceNumber', $new_sequence, ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
    }
  }
  $path = drupal_get_path('module', 'islandora_book');
  drupal_add_css("$path/css/islandora_book.css");
}