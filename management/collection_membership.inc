<?php

function book_collection_management(&$form_state, $pid) {

  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $potential_collections = get_collections_as_option_array();
  $current_collections = get_collection_relationships($pid);
  $keys = array_keys($current_collections);
  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );

  $form['collections'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Collection Membership'),
  );

  $form['collections']['membership'] = array(
    '#title' => 'Collection Membership',
    '#description' => t("Books will belong to all selected collections"),
    '#type' => 'checkboxes',
    '#options' => $potential_collections,
    '#default_value' => $keys,
  );

  $form['collections']['submit'] = array(
    '#type' => 'submit',
    '#id' => 'add_collection',
    '#value' => t('Save Book Membership'),
  );

  return $form;
}

function book_collection_management_validate($form, &$form_state) {

  if (count(array_unique(array_values($form_state['values']['membership']))) == 1) {
    form_set_error('membership', "Object must be a member of at least one collection");
    return;
  }
}

/**
 * book book management form submit
 * @param array $form
 * @param array $form_state
 */
function book_collection_management_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  global $user;
  $book_pid = $form_state['values']['pid'];
  $current_collections = array_keys(get_collection_relationships($book_pid));
  $new_collections = array_values(array_filter($form_state['values']['membership'], function($e) {
                return ($e !== 0);
              }));

  $to_keep = array_intersect($current_collections, $new_collections);
  $to_toss = array_diff($current_collections, $to_keep);
  $to_add = array_diff($new_collections, $to_keep);

  $item = new Fedora_Item($book_pid);
  if (!empty($to_toss)) {
    foreach ($to_toss as $relationship) {
      $item->purge_relationship('isMemberOfCollection', $relationship);
    }
  }
  if (!empty($to_add)) {
    foreach ($to_add as $relationship) {
      $item->add_relationship('isMemberOfCollection', $relationship, RELS_EXT_URI);
    }
  }
  drupal_goto('fedora/repository/' . $book_pid);
}
