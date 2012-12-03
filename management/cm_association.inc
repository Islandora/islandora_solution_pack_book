<?php

function book_content_model_association(&$form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'ContentModel');
  $cm = ContentModel::loadFromObject($pid);
  $options = array(
    'islandora:bookCModel' => 'IIV',
    'islandora:iaBookCModel' => "Internet Archive Viewer"
  );

  $current_model = $cm->pid;
  $chosen = 'islandora:bookCModel';
  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['cm'] = array(
    '#type' => 'hidden',
    '#value' => $current_model,
  );

  $form['viewer'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Select Viewer'),
  );

  $form['viewer']['membership'] = array(
    '#title' => 'Select Viewer',
    '#type' => 'radios',
    '#options' => array('islandora:bookCModel' => 'IIV', 'islandora:iaBookCModel' => "Internet Archive Viewer"),
    '#default_value' => $current_model,
  );

  $form['viewer']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Choose Viewer'),
  );

  return $form;
}

/**
 * book book management form submit
 * @param array $form
 * @param array $form_state
 */
function book_content_model_association_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $book_pid = $form_state['values']['pid'];
  $current_model = $form_state['values']['cm'];
  $new_model = $form_state['values']['membership'];
  if($current_model != $new_model){
      $item = new Fedora_Item($book_pid);
      $item->purge_relationship('hasModel', $current_model, FEDORA_MODEL_URI);
      $item->add_relationship('hasModel', $new_model, FEDORA_MODEL_URI);
  }

  drupal_goto('fedora/repository/' . $book_pid);
}
