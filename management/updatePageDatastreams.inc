<?php

function islandora_book_update_derived($form_state, $pid) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  global $base_url;
  $languages = islandora_book_get_available_tesseract_languages();

  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to page view'), "fedora/repository/$pid"),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['page_manage'] = array(
    '#type' => 'item',
    '#title' => t('Update Derived Datastreams'),
    '#description' => t('Update datastreams for this page object.  The orginal stored tiff will be used to create all derived datastreams.'),
  );
  $form['do_ocr'] = array(
    '#type' => 'checkbox',
    '#title' => t('Refresh OCR for this page'),
    '#description' => t('Refreshing the OCR will remove all edits to the previous OCR stream, including TEI and entity markup.'),
  );

  $form['preprocess'] = array(
    '#title' => t('Preprocessing for typescripts?'),
    '#description' => t('Will add additional processing for typewritten text.  This script grayscales and sharpens an ingested .tiff before performing OCR processes. <br />
        This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive. <br />
        If your image is already gray, this script will not run'),
    '#type' => 'checkbox',
  );

  $form['language'] = array(
    '#title' => t('Language'),
    '#type' => 'select',
    '#options' => $languages,
    '#selected' => 'eng',
  );

  $form['recreate_derivatives'] = array(
    '#type' => 'checkbox',
    '#title' => t('Refresh all derived images')
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Page Datastreams'),
  );

  return $form;
}

function islandora_book_update_derived_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  global $user;
  $pid = $form_state['values']['pid'];
  $item = new Fedora_Item($pid);
  $tiff_file_path = save_datastream_as_tempfile($pid, 'TIFF');
  $language = $form_state['values']['language'];
  $preprocess = $form_state['values']['preprocess'];
  if ($form_state['values']['do_ocr']) {
    $success = refresh_ocr($tiff_file_path, $item, $language, $preprocess);
    if ($success) {
      drupal_set_message(t("OCR refreshed for this page"));
    }
  }
  if ($form_state['values']['recreate_derivatives']) {
    $success = refresh_images($tiff_file_path, $item);
    drupal_set_message("Refreshed $success of 3 image files for this page.");
  }
  file_delete($tiff_file_path);
  drupal_goto("fedora/repository/$pid");
}

