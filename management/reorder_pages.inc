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
      '#description' => t("Checked pages will be deleted. <strong>THIS CANNOT BE UNDONE!</strong><br />Drag individual pages to resequence book."),
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
      '#value' => t('Save Page Reordering'),
    );

    $form['reorder_pages']['confirm']['submit'] = array(
      '#id' => 'delete_pages',
      '#type' => 'submit',
      '#value' => t('Delete Selected Pages'),
    );
  }
  return $form;
}

function page_reorder_submit($form, &$form_state) {

  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  if ($form_state['clicked_button']['#id'] == 'reorder_pages') {
    $pages = $form_state['values']['table']['rows'];
    foreach ($pages as $page) {
      $new_sequence = ($page['pos'] + 1) / 2;
      if ($new_sequence != $page['current_sequence']) {
        $item = new Fedora_Item($page['page_pid']);
        $item->purge_relationships('isSequenceNumber', $page['current_sequence'], ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
        $item->add_relationship('isSequenceNumber', $new_sequence, ISLANDORA_RELS_EXT_URI, RELS_TYPE_PLAIN_LITERAL);
      }
    }
  }
  if ($form_state['clicked_button']['#id'] == 'delete_pages') {
    $book_pid = $form_state['values']['pid'];
    $page_pids = @array_filter($form_state['values']['table']['selections']);
    $count = count($page_pids);
    $batch = array(
      'title' => t("Deleting @page_count pages", array('@page_count' => $count)),
      'operations' => array(),
      'file' => drupal_get_path('module', 'islandora_book') . '/management/add_pages.inc',
      'init_message' => t('Preparing data...'),
      'progress_message' => t('@current of @total tasks completed.'),
    );

    foreach ($page_pids as $page_pid) {
      $batch['operations'][] = array('islandora_book_delete_single_page', array($page_pid));
    }
    $batch['operations'][] = array('post_deletion_renumber_pages_wrapper', array($book_pid));
    batch_set($batch);
    batch_process("fedora/repository/$book_pid");

    $path = drupal_get_path('module', 'islandora_book');
  }
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