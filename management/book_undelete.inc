<?php

function book_undelete_form(&$form_state) {
  module_load_include('inc', 'islandora_book', 'management/IslandoraBookUndelete');
  $table = islandora_book_undelete_table();

  $form['table'] = array(
    '#type' => 'markup',
    '#value' => t("There are no inactive books in this repository"),
  );

  if (user_access('undelete books') && $table) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    $form['table'] = array(
      'table' => $table,
    );
    $form['activate']['submit'] = array(
      '#id' => 'activate',
      '#type' => 'submit',
      '#value' => t('Restore Books to Active State'),
    );
    $form['delete']['submit'] = array(
      '#id' => 'remove',
      '#type' => 'submit',
      '#value' => t('Permanently Delete Books'),
    );
  }
  return $form;
}

function book_undelete_form_submit($form, &$form_state) {
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $to_process = @array_filter($form_state['values']['table']['selections']);
  $count = count($to_process);
  if ($count == 0) {
    return;
  }
  $process = $form_state['clicked_button']['#id'];
  if ($process == 'activate') {
    $verb = t("Restoring");
  }
  else {
    $verb = t("Deleting");
  }

  $batch = array(
    'title' => t("@verb @book_count book(s)", array('@verb' => $verb, '@book_count' => $count)),
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/management/book_undelete.inc',
    'init_message' => t('Preparing data...'),
    'progress_message' => t('@current of @total tasks completed.'),
  );

  if ($form_state['clicked_button']['#id'] == 'activate') {
    foreach ($to_process as $book_pid) {
      $batch['operations'][] = array('restore_inactive_book', array($book_pid));
    }
  }
  if ($form_state['clicked_button']['#id'] == 'remove') {
    foreach ($to_process as $book_pid) {
      $batch['operations'][] = array('delete_inactive_book', array($book_pid));
    }
  }
  batch_set($batch);
}

function delete_inactive_book($book_pid) {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $page_pids = get_sorted_pages($book_pid, 'Inactive');
  if (!empty($page_pids)) {
    foreach ($page_pids as $page_pid) {
      $item = new Fedora_Item($page_pid);
      $item->purge("purged by Admin");
      $item->forget();
    }
  }
  $item = new Fedora_Item($book_pid);
  $item->purge("purged by Admin");
  $item->forget();
}

function restore_inactive_book($book_pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $item = new Fedora_Item($book_pid);
  $item->modify_object($item->objectProfile->objLabel, 'A', $user->uid, 'State restored by Admin');
  $page_pids = get_sorted_pages($book_pid, 'Inactive');
  if (!empty($page_pids)) {
    foreach ($page_pids as $page_pid) {
      $item = new Fedora_Item($page_pid);
      $item->modify_object($item->objectProfile->objLabel, 'A', $user->uid, 'State restored by Admin');
      $item->forget();
    }
  }
}