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
      '#title' => t("Manage Book Pages"),
      '#description' => t("Checked pages will be deleted. <strong>THIS CANNOT BE UNDONE!</strong><br />Drag individual pages to resequence book.<br />Clicking on the page image will open the page object in new tab."),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['reorder_pages']['table'] = array(
      'table' => $table,
    );

    $form['reorder_pages']['reorder_pages']['submit'] = array(
      '#id' => 'reorder',
      '#type' => 'submit',
      '#value' => t('Save Changes'),
    );
  }
  return $form;
}

function page_reorder_submit($form, &$form_state) {
  $book_pid = $form_state['values']['pid'];
  $pids_to_delete = @array_filter($form_state['values']['table']['selections']);
  $pages = $form_state['values']['table']['rows'];
  $pageholder = array();
  foreach($pages as $page){
    $arraypos = intval($page['pos']);
    $pageholder[$arraypos] = $page;
  }
  ksort($pageholder);
  $pageholder = array_values($pageholder);
  $stop = "";
  $batch = array(
    'title' => t("Processing @page_count pages", array('@page_count' => $count)),
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/management/reorder_pages.inc',
    'init_message' => t('Preparing data...'),
    'progress_message' => t('@current of @total tasks completed.'),
  );


  //reorder pages first
  foreach ($pageholder as $index => $page) {
    $new_sequence = $index + 1;
    if ($new_sequence != $page['current_sequence']) {
      $batch['operations'][] = array('reorder_dragged_pages', array($page['page_pid'], $page['current_sequence'], $new_sequence));
    }
    if ($page['label'] != $page['current_label']) {
      $batch['operations'][] = array('relabel_pages', array($page['page_pid'], $page['label']));
    }
  }
  // delete selected pages

  foreach ($pids_to_delete as $page_pid) {
    $batch['operations'][] = array('islandora_book_delete_single_page_wrapper', array($page_pid));
  }
  $batch['operations'][] = array('post_deletion_renumber_pages_wrapper', array($book_pid));

  islandora_book_batch_process_pages($batch);
}

/*
 * This function is necessary as the Drupal batch api runs out of memory when being called
 * from a large form submit handler
 */
function islandora_book_batch_process_pages($batch) {
  batch_set($batch);
  batch_process();
}

/**
 * Wrapper to call function from utilities page
 * Batch will not call functions from multiple files
 * @param string $book_pid
 */
function post_deletion_renumber_pages_wrapper($book_pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  post_deletion_renumber_pages($book_pid);
}

function islandora_book_delete_single_page_wrapper($page_pid) {
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  islandora_book_delete_single_page($page_pid);
}

function reorder_dragged_pages($pid, $old_sequence, $new_sequence) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  $item->purge_relationships('isSequenceNumber', $old_sequence, ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
  $item->add_relationship('isSequenceNumber', $new_sequence, ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
}

function relabel_pages($pid, $label) {
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);

  // update dc stream with new label
  $dc = $item->get_datastream_dissemination('DC');
  $xml = simplexml_load_string($dc);
  $xml->title[0] = $label;
  $dc = $xml->asXML();
  $item->modify_datastream($dc, 'DC', 'DC', 'text/xml');

  // if MODS stream exits, update it
  if (key_exists('MODS', $item->datastreams)) {
    $mods = $item->get_datastream_dissemination('MODS');
    $modsxml = simplexml_load_string($mods);
    $modsxml->titleInfo->title = $label;
    $mods = $modsxml->asXML();
    $item->modify_datastream($mods, 'MODS', 'MODS', 'text/xml');
  }

  // update object label
  $item->modify_object($label, 'A', $user->uid);
}