<?php

/**
 * @file
 *
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_admin_settings_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $get_default_value = function($name, $default) use(&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $tesseract = $get_default_value('islandora_book_tesseract', '/usr/bin/tesseract');
  $gimp = $get_default_value('islandora_book_gimp', '/usr/bin/gimp');
  $gs = $get_default_value('islandora_book_gs', '/usr/bin/gs');
  $languages = $get_default_value('islandora_book_tesseract_enabled_languages', drupal_map_assoc(array_keys(islandora_book_get_tesseract_languages())));
  $form = array(
    'solr_settings' => array(
      // @todo The Solr fields are not in active use, make them inaccessible for now.
      '#access' => FALSE,
      '#disabled' => !module_exists('islandora_solr'),
      '#type' => 'fieldset',
      '#title' => t('SOLR Settings'),
      '#description' => t('Various Solr fields for use to organize books. <b>Requires</b> <i>Islandora Solr</i> to be enabled.'),
      'islandora_book_solr_subject_field' => array(
        '#type' => 'textfield',
        '#title' => t('Subject field'),
        '#description' => t('The SOLR field containing the untokenized subject value, used to group books with the same subject.'),
        '#default_value' => $get_default_value('islandora_book_solr_subject_field', 'mods.subject')
      ),
    ),
    'local_derivative_settings' => array(
      '#type' => 'fieldset',
      '#title' => t('Create Page Derivatives Locally'),
      '#description' => t('When ingesting pages automatically create derivative datastreams on this server. If you intend to only use microservices to generate derivatives you should not have any of these settings enabled.'),
      'islandora_book_ingest_derivatives' => array(
        '#type' => 'checkboxes',
        '#element_validate' => array('islandora_book_admin_settings_form_ingest_derivatives_validate'),
        '#options' => array(
          'pdf' => t('PDF datastream. <b>Requires</b> <i>ImageMagick</i>'),
          'image' => t('Image datastreams (TN, JPEG, JP2). <b>Requires</b> <i>Large Image Solution Pack</i>'),
          'ocr' => t('OCR datastreams (OCR, RAW_HOCR, ENCODED_OCR)')
        ),
        '#default_value' => $get_default_value('islandora_book_ingest_derivatives', drupal_map_assoc(array('pdf', 'image', 'ocr')))
      ),
      'pdf_derivative_settings' => array(
        '#type' => 'fieldset',
        '#title' => t('PDF Settings'),
        'islandora_book_gs' => array(
          '#type' => 'textfield',
          '#title' => t('GS (GhostScript)'),
          '#description' => t('GhostScript is used to combined PDF files and generate a PDF repersentation of a Book.<br/>!msg',
                          array('!msg' => islandora_book_admin_settings_form_executable_availible_message($gs))),
          '#default_value' => $gs,
          '#prefix' => '<div id="gimp-wrapper">',
          '#suffix' => '</div>',
          '#ajax' => array(
            'callback' => 'islandora_book_admin_settings_form_gimp_ajax_callback',
            'wrapper' => 'gimp-wrapper',
            'effect' => 'fade',
            'event' => 'change'
          )
        ),
      ),
      'ocr_derivative_settings' => array(
        '#type' => 'fieldset',
        '#title' => t('OCR Settings'),
        'islandora_book_tesseract' => array(
          '#type' => 'textfield',
          '#title' => t('Tesseract'),
          '#description' => t('Tesseract is used to generate the OCR datastreams.<br/>!msg', array('!msg' => islandora_book_admin_settings_form_executable_availible_message($tesseract))),
          '#default_value' => $tesseract,
          '#prefix' => '<div id=oto-line"tesseract-wrapper">',
          '#suffix' => '</div>',
          '#ajax' => array(
            'callback' => 'islandora_book_admin_settings_form_tesseract_ajax_callback',
            'wrapper' => 'tesseract-wrapper',
            'effect' => 'fade',
            'event' => 'change'
          ),
        ),
        'islandora_book_gimp' => array(
          '#type' => 'textfield',
          '#title' => t('Gimp'),
          '#description' => t('Gimp is used to preprocess the source image before generating the OCR datastreams.<br/>!msg',
                          array('!msg' => islandora_book_admin_settings_form_executable_availible_message($gimp))),
          '#default_value' => $gimp,
          '#prefix' => '<div id="gimp-wrapper">',
          '#suffix' => '</div>',
          '#ajax' => array(
            'callback' => 'islandora_book_admin_settings_form_gimp_ajax_callback',
            'wrapper' => 'gimp-wrapper',
            'effect' => 'fade',
            'event' => 'change'
          )
        ),
        'islandora_book_tesseract_enabled_languages' => array(
          '#type' => 'checkboxes',
          '#title' => t('Languages available for OCR'),
          '#options' => islandora_book_get_tesseract_languages(),
          '#description' => t('Select from the language packs available on your
                             processing server.<br/> These are normally found in
                             <strong>/usr/local/share/tessdata/</strong><br/>
                             Check with your systems admininistrator if you are
                             unsure of availablilty'),
          '#default_value' => $languages,
        )
      )
    )
  );
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $form += islandora_viewers_form('islandora_book_viewers', NULL, 'islandora:bookCModel');
  $form['book_viewers'] = $form['viewers'];
  $form['book_viewers']['#title'] = t('Book Viewers');
  unset($form['viewers']);
  $form += islandora_viewers_form('islandora_book_page_viewers', 'image/jp2');
  $form['page_viewers'] = $form['viewers'];
  $form['page_viewers']['#title'] = t('Page Viewers');
  unset($form['viewers']);
  return system_settings_form($form);
}

/**
 * Check if the required resouces are enabled.
 *
 * @param array $element
 *   The element to check.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_book_admin_settings_form_ingest_derivatives_validate(array $element, array &$form_state, array $form) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  // form_error() doesn't seem to work here, it prevents the submit but
  // also prevents the value from being set. So we are using
  // drupal_set_message().
  if ($element['#value']['pdf'] && !islandora_book_can_create_pdf()) {
    $element['#value']['pdf'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('ImagicMagick must be enabled to derive the PDF datastream.'), 'error');
  }
  if ($element['#value']['image'] && !islandora_book_can_create_images()) {
    $element['#value']['image'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('The Large Image solution pack must be enabled to derive image datastreams.'), 'error');
  }
}

/**
 * Gets a message to display if the ocr program provided is accessible or not.
 *
 * @param string $path
 *   The path to an executable to check for availability.
 *
 * @return string
 *   A message in html detailing if the given executable is accessible.
 */
function islandora_book_admin_settings_form_executable_availible_message($path) {
  $available = is_executable($path);
  if ($available) {
    $image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $message = t('Executable found at @path', array('@path' => $path));
  }
  else {
    $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $message = t('Unable to locate executable at @path', array('@path' => $path));
  }
  return $image . $message;
}

/**
 * Ajax callback for the Tesseract textfield.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_book_admin_settings_form_tesseract_ajax_callback(array $form, array $form_state) {
  return $form['local_derivative_settings']['ocr_derivative_settings']['islandora_book_tesseract'];
}

/**
 * Ajax callback for the Gimp textfield.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_book_admin_settings_form_gimp_ajax_callback(array $form, array $form_state) {
  return $form['local_derivative_settings']['ocr_derivative_settings']['islandora_book_gimp'];
}
