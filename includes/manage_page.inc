<?php

/**
 * @file
 * The page management menu.
 */

/**
 * Displays all the page management forms in a set of vertical tabs.
 *
 * @param FedoraObject $object
 *   The page object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   page objects.
 */
function islandora_book_manage_page_menu(FedoraObject $object) {
  return array(
    'manage_page' => array(
      '#type' => 'vertical_tabs',
      'pdf' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Create a PDF'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_page_pdf_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'ocr' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Perform OCR'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_page_ocr_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      ),
      'images' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Create Images'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_page_images_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
    ),
  );
}

/**
 * Updates this objects derived OCR datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object to be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_manage_page_ocr_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_book_can_derive($object, 'OCR');
  $can_preprocess = islandora_book_can_preprocess_ocr();
  $languages = islandora_book_get_enabled_tesseract_languages();
  return array(
    'language' => array(
      '#title' => t('Language'),
      '#type' => 'select',
      '#description' => t('Please select the language the page is written in.'),
      '#options' => $languages,
    ),
    'preprocess' => array(
      '#disabled' => !$can_preprocess,
      '#title' => t('Preprocessing for typescripts?'),
      '#type' => 'checkbox',
      '#description' => t('Will add additional processing for typewritten text. This script grayscales and sharpens the archival TIFF before performing OCR processes.<br/>This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive.<br/>'),
      '#default_value' => $can_preprocess,
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Perform OCR'),
    ),
  );
}

/**
 * Submit handler for the manage page OCR form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_page_ocr_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $form_state['object'];
  $options = array(
    'language' => $form_state['values']['language'],
    'preprocess' => (bool) $form_state['values']['preprocess'],
  );
  islandora_book_page_derive_ocr_datastreams($object, $options);
}

/**
 * Derives the given pages objects image datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object to be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_manage_page_images_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_book_can_derive($object, 'JP2');
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>Large Image Solution Pack</b> installed to create images.<br/>This will update the TN, JPG, and JP2 datastreams.'),
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Create Images'),
    ),
  );
}

/**
 * Submit handler for the manage page images form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_page_images_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $form_state['object'];
  islandora_book_page_create_image_derivatives($object);
}

/**
 * Derives the given page objects PDF datastream.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object to be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_manage_page_pdf_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_book_can_derive($object, 'PDF');
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>ImageMagick</b> installed to create PDF files.'),
    ),
    'dpi' => array(
      '#type' => 'select',
      '#title' => t('DPI (Dots Per Inch)'),
      '#description' => t('Set the DPI for the generated PDF.'),
      '#options' => drupal_map_assoc(array('72', '96', '300', '600')),
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Create PDF'),
    ),
  );
}

/**
 * Submit handler for the manage page pdf form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_page_pdf_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $form_state['object'];
  $options = array(
    '-density' => $form_state['values']['dpi'],
    '-compress' => 'LZW',
  );
  islandora_book_page_derive_pdf_datastream($object, $options);
}
