<?php

/**
 * @file
 * Contains one local action for ingesting pages, and one local task which
 * contains several forms for managing book objects and their pages.
 *
 * Management Forms:
 *  Create PDF.
 *  Create OCR.
 *  Create Images.
 *  Update Book Thumbnail.
 *  Reorder Pages.
 *  Delete Pages.
 */

/**
 * Local menu action, that gets the ingest page form.
 *
 * @param FedoraObject $object
 *   The book to ingest into.
 *
 * @return sting
 *   The HTML repersentation of the ingest page form.
 */
function islandora_book_ingest_page(FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  drupal_set_title(t('Add page to @book', array('@book' => $object->label)));
  return drupal_get_form('islandora_ingest_form', array(
      'book' => $object->id,
      'namespace' => islandora_get_namespace($object->id),
      'models' => array('islandora:pageCModel'),
    )
  );
}

/**
 * Displays all the book management forms in a set of vertical tabs.
 *
 * @param FedoraObject $object
 *   The book object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   book objects.
 */
function islandora_book_manage_book_menu(FedoraObject $object) {
  drupal_set_title($object->label);
  return array(
    'manage_book' => array(
      '#type' => 'vertical_tabs',
      '#weight' => 0,
      '#prefix' => '',
      'pdf' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Create PDF'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_book_pdf_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      ),
      'ocr' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Perform OCR'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_book_ocr_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'images' => array(
        '#access' => user_access(FEDORA_ADD_DS),
        '#title' => t('Create Images'),
        '#type' => 'fieldset',
        'form_1' => drupal_get_form('islandora_book_manage_book_thumbnail_form', $object),
        'form_2' => drupal_get_form('islandora_book_manage_book_images_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'sequence' => array(
        '#access' => user_access(FEDORA_METADATA_EDIT),
        '#title' => t('Reorder Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_book_sequences_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'delete' => array(
        '#access' => user_access(FEDORA_PURGE),
        '#title' => t('Delete Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_manage_book_delete_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
    ),
  );
}

/**
 * Creates a PDF of the book.
 *
 * Generates a PDF file for each page and then combines them into a single PDF.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_pdf_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_paged_content_can_create_pdf() && islandora_paged_content_can_combine_pdf();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>ImageMagick</b> and <b>GhostScript</b> installed to create/combine PDF files.<br/> This will also update the PDF datastreams for each Page object.'),
    ),
    'dpi' => array(
      '#type' => 'select',
      '#title' => t('DPI - Dots Per Inch'),
      '#description' => t('Set the DPI for the generated PDF. The higher the resolution the longer the process will take.'),
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
 * Creates a PDF of the book and a PDF of every page.
 *
 * Triggers a batch to derive a PDF datastreams in each page object.then
 * combined them into a single PDF which is stored in the book object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_pdf_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_paged_content_get_pages($object));
  $options = array(
    '-density' => $form_state['values']['dpi'],
    '-compress' => 'LZW',
  );
  $batch = islandora_paged_content_create_pdf_batch($object, $pages, $options);
  batch_set($batch);
}

/**
 * Derives the OCR datastreams in each page of the book.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_ocr_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_ocr', 'includes/utilities');
  $form_state['object'] = $object;
  if (module_exists('islandora_ocr')) {
    $can_derive = islandora_ocr_can_derive_ocr();
    $languages = islandora_ocr_get_enabled_tesseract_languages();
    return array(
      'description' => array(
        '#type' => 'item',
        '#description' => t('You must have the <b>Tesseract</b> installed to perform OCR, and <b>Gimp</b> to preprocess OCR.<br/> This will update the OCR and HOCR datastreams for each Page object.'),
      ),
      'language' => array(
        '#title' => t('Language'),
        '#type' => 'select',
        '#description' => t('Please select the language that the page is written in.'),
        '#options' => $languages,
      ),
      'submit' => array(
        '#disabled' => !$can_derive,
        '#type' => 'submit',
        '#value' => t('Create OCR'),
      ),
    );
  }
  else {
    return array(
      'sorry' => array(
        '#type' => 'item',
        '#markup' => t('The OCR module must be installed and enabled in ' .
          'order for this form to be used.'),
      ),
    );
  }
}

/**
 * Triggers a batch to derive the OCR datastreams in each page object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_ocr_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_paged_content_get_pages($object));
  $options = array(
    'language' => $form_state['values']['language'],
    'preprocess' => FALSE,
  );
  $batch = islandora_paged_content_create_ocr_batch($object, $pages, $options);
  batch_set($batch);
}

/**
 * Updates the books thumbnail from the first page in the book.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to update.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_thumbnail_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $form_state['object'] = $object;
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t("Update the Book's thumbnail image. The book must have pages and the first page must have a TN datastream."),
    ),
    'submit' => array(
      '#disabled' => !islandora_paged_content_can_update_paged_content_thumbnail($object),
      '#type' => 'submit',
      '#value' => t('Update Thumbnail'),
    ),
  );
}

/**
 * The submit handler for the update thumbnail form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_thumbnail_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  if (islandora_paged_content_update_paged_content_thumbnail($form_state['object'])) {
    drupal_set_message(t('Thumbnail successfully updated.'), 'status');
  }
  else {
    drupal_set_message(t('Failed to update thumbnail'), 'error');
  }
}

/**
 * Derives the image datastreams for each page object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_images_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_paged_content_can_create_images();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>Large Image Solution Pack</b> installed to perform create images.<br/> This will update the TN, JPG, and JP2 datastreams for each Page object.'),
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Create Images'),
    ),
  );
}

/**
 * Triggers a batch to derive image datastreams in each page object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_images_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_paged_content_get_pages($object));
  $batch = islandora_paged_content_create_images_batch($object, $pages);
  batch_set($batch);
}

/**
 * Form for reordering the books pages.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The book object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_sequences_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $form_state['object'] = $object;
  $pages = islandora_paged_content_get_pages($object);
  $delta = count($pages);
  $map = function($page) use($delta) {
    return array(
      'pid' => array('#markup' => check_plain($page['pid'])),
      'label' => array('#markup' => check_plain($page['label'])),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => $page['page'],
        '#delta' => $delta,
        '#title-display' => 'invisible',
      ));
  };
  $rows = array_map($map, $pages);
  return array(
    '#tree' => TRUE,
    'table' => array(
      '#type' => 'markup',
      '#header' => array(t('PID'), t('Label'), t('Page')),
      'rows' => $rows,
    ),
    'actions' => array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Save Changes'),
      ),
    ),
  );
}

/**
 * Submit handler for the sequence form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_sequences_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  $object = $form_state['object'];
  $pages = &$form_state['values']['table']['rows'];
  if ($pages) {
    uasort($pages, 'drupal_sort_weight');
    $pages = array_keys($pages);
    $batch = islandora_paged_content_sequence_pages_batch($object, $pages);
    batch_set($batch);
  }
}

/**
 * Gets the delete pages form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_manage_book_delete_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $form_state['object'] = $object;
  $pages = islandora_paged_content_get_pages($object);
  return array(
    'table' => array(
      '#type' => 'tableselect',
      '#header' => array(
        'pid' => t('PID'),
        'page' => t('Sequence Number'),
        'label' => t('Label'),
      ),
      '#options' => $pages,
      '#multiple' => TRUE,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Delete Selected Pages'),
    ),
  );
}

/**
 * Submit handler for the delete pages form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_manage_book_delete_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  $object = $form_state['object'];
  $pages = array_values(array_filter($form_state['values']['table']));
  $batch = islandora_paged_content_delete_pages_batch($object, $pages);
  batch_set($batch);
}
