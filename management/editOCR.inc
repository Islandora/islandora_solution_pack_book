<?php

/**
 * builds form
 * @param array form_state
 * @param string pid
 * @return array
 */
function islandora_edit_ocr(&$form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $item = new Fedora_Item($pid);
  $ocr = $item->get_datastream_dissemination('OCR');
  $alink_options = array(
    'attributes' => array('target' => '_blank', 'class' => 'fedora_image', 'title' => $pid),
    'html' => TRUE,
  );

  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to page view'), "fedora/repository/$pid"),
  );
  $form['link2'] = array(
    '#type' => 'item',
    '#value' => l(t('Open Image in New Tab'), "fedora/repository/$pid/JPEG", $alink_options),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['editarea'] = array(
    '#type' => 'textarea',
    '#default_value' => $ocr,
    '#rows' => 40
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Text'),
  );

  return $form;
}

function islandora_edit_ocr_submit($form, &$form_state) {
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $ocr = $form_state['values']['editarea'];
  $item = new Fedora_Item($pid);
  $item->modify_datastream($ocr, 'OCR', 'Scanned Text', 'text/plain');
}