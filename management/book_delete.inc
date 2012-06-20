<?php

/**
 * builds form
 * @param array form_state
 * @param string pid
 * @return array
 */
function book_delete(&$form_state, $pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $count = count(get_collection_relationships($pid));
  $description = t('Clicking this button will delete this book. <br /> <strong>This action cannot be undone.</strong>');
  if ($count > 1) {
    $description = t("This book is a member of @count collections !break
      Books with membership in multiple collections cannot be deleted. !break
      Click on <strong>Collection Membership</strong> above to remove book from additional collections."
            , array('@count' => $count, '!break' => '<br />'));
    $multiple = TRUE;
  }

  if (user_access('delete pages')) {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    $form['delete_book'] = array(
      '#title' => "Permanently Delete this Book",
      '#type' => 'fieldset',
      '#description' => $description,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    if (!$multiple) {
      $form['delete_book']['confirm'] = array(
        '#title' => "Are you sure?",
        '#type' => 'fieldset',
        '#description' => t('<strong>Clicking the delete button will permanantly remove this book and all of its pages.</strong> <br />  <strong>This action cannot be undone.</strong> '),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $form['delete_book']['confirm']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Delete this book'),
        '#id' => 'delete_page',
      );
    }
  }

  return $form;
}

/**
 * book book management form submit
 * @param array $form
 * @param array $form_state
 */
function book_delete_submit($form, &$form_state) {
  global $user;
  $book_pid = $form_state['values']['pid'];
  if ($form_state['clicked_button']['#id'] == 'delete_page') {
    $page_pids = get_page_pids($book_pid);
    foreach ($page_pids as $page_pid) {
      $item = new Fedora_Item($page_pid);
      $item->purge($pid . ' deleted by user identified as ' . $user->name);
    }
    $item = new Fedora_Item($book_pid);
    $item->purge($book_pid . ' deleted by user identified as ' . $user->name);
  }
  drupal_goto('fedora/repository/' . $collection_pids[0]);
}

