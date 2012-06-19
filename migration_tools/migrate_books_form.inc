<?php

function migrate_books_form(&$form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['migrate_book'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Add new Islandora relationships to book objects'),
    '#description' => t('The Islandora Book solution pack has added some new internal relationships.  Add them here.'),
  );

  $form['migrate_book']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update book objects')
  );


  return($form);
}



function migrate_books_form_submit($form, &$form_state) {
  $collection_pid = $form_state['values']['pid'];
  batch_process_book($collection_pid);
}

