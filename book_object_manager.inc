<?php

/**
 * wrapper function to allow tab callback detail to be called from url
 * @param string pid
 * @return boolean
 */
function book_management_wrapper($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  return drupal_render(get_page_model_management_content($pid));
}

/**
 * Supplies tab contents to callback
 * @param string pid
 * @return array
 */
function get_book_model_management_content($pid) {
  if (!user_access('manage book object')) {
    return;
  }

  module_load_include('inc', 'islandora_book', 'management/collection_membership');
  module_load_include('inc', 'islandora_book', 'management/view_metadata');
  module_load_include('inc', 'islandora_book', 'management/book_datastreams');
  module_load_include('inc', 'islandora_book', 'management/book_delete');
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  $content = drupal_get_form('book_add_pages', $pid);
  $content .= drupal_get_form('book_collection_management', $pid);
  $content .= drupal_get_form('view_metadata', $pid);
  $content .= drupal_get_form('update_datastreams', $pid);
  $content .= drupal_get_form('current_datastreams', $pid);
 
  $content .= drupal_get_form('page_delete', $pid);
  $content .= drupal_get_form('book_delete', $pid);
  $selected = FALSE;
  if ($_SESSION['manage_chosen']){
    $selected = TRUE;
    unset ($_SESSION['manage_chosen']);
  }
  $form['manage_book_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Manage'),
    '#content' => $content,
    '#selected' => $selected,
    '#weight' => 100
    
  );
  return $form;
}

