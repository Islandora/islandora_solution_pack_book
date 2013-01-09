<?php


/**
 * Wrapper to return management content as tab
 * @param PID $pid
 * @return rendered form
 */
function page_management_wrapper($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  return drupal_render(get_page_model_management_content($pid));
}


/**
 *
 * @param PID $pid
 * @return form
 */
function get_page_model_management_content($pid) {
  $page_management_links = array();
  $page_management_links[] = l(t('Metadata'), "islandora/page/manage/metadata/$pid");
  $page_management_links[] = l(t('Update Derived Datastreams'), "islandora/page/manage/refresh/$pid");
  $page_management_links[] = l(t('Manage Current Datastreams'), "islandora/page/manage/datastreams/$pid");
  $page_management_links[] = l(t('Edit OCR'), "islandora/page/manage/editOCR/$pid");
  if (user_access('delete pages')) {
    $page_management_links[] = l(t('Delete This Page'), "islandora/page/manage/delete/$pid");
  }
  $list = theme_item_list($page_management_links);
  $form['manage_page_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Manage'),
    '#content' => $list,
    '#selected' => FALSE,
  );
  return $form;
}


function islandora_book_page_management($function, $pid) {
  module_load_include('inc', 'islandora_book', 'management/pageMetadata');
  module_load_include('inc', 'islandora_book', 'management/updatePageDatastreams');
  module_load_include('inc', 'islandora_book', 'management/managePageDatastreams');
  module_load_include('inc', 'islandora_book', 'management/editOCR');
  $mappings = array(
    'metadata' => 'islandora_book_get_metadata_link',
    'refresh' => 'islandora_book_update_derived',
    'datastreams' => 'islandora_book_page_datastreams',
    'delete' => 'islandora_book_page_delete',
    'editOCR' => 'islandora_edit_OCR'
  );
  $results = drupal_get_form($mappings[$function], $pid);
  return drupal_get_form($mappings[$function], $pid);
}