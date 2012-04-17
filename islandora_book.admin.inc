<?php

/**
 * @file 
 * Islandora book admin 
 */

/**
 * islandora book admin settings
 * @param type $form_state
 * @return type 
 */
function islandora_book_admin_settings(&$form_state) {


  $ocr_path = isset($form_state['values']['islandora_ocr_path']) ? $form_state['values']['islandora_ocr_path'] : variable_get('islandora_ocr_path', '/usr/bin/tesseract');
  $ocr_avail = exec("ls $ocr_path");
  $confirmation_message = ($ocr_avail ? '<img src="' . url('misc/watchdog-ok.png') . '"/>'
          . t('Executable found at !ocr_path', array('!ocr_path' => $ocr_path)) : '<img src="'
          . url('misc/watchdog-error.png') . '"/> '
          . t('Unable to locate executable at !ocr_path</p>', array('!ocr_path' => $ocr_path)));

  $ocr = isset($form_state['values']['islandora_book_do_ocr']) ? $form_state['values']['islandora_book_do_ocr'] : variable_get('islandora_book_do_ocr', TRUE);
  $help_page = l('Help', 'admin/help/islandora_book');

  $form = array();

  $form['info'] = array(
    '#type' => 'item',
    '#value' => $help_page,
  );


  $form['islandora_book_create_images'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create derivative images locally?'),
    '#description' => t('Leave this box checked unless processing of images is done on an external server.'),
    '#default_value' => variable_get('islandora_book_create_images', TRUE),
  );

  $form['islandora_create_book_pdf'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create PDF locally ?'),
    '#description' => t('Check this box to have a PDF produced locally on ingest.'),
    '#default_value' => variable_get('islandora_create_book_pdf', FALSE),
  );
    $form['book_ahah_wrapper'] = array(
    '#prefix' => '<div id="ibook-url">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
  );
  $form['book_ahah_wrapper']['islandora_book_do_ocr'] = array(
    '#type' => 'checkbox',
    '#title' => t('Perform OCR on incoming TIFF images?'),
    '#description' => t('Do not check this box if OCR is performed on an external server, such as ABBYY'),
    '#default_value' => $ocr,
    '#ahah' => array(
      'path' => 'islandora/book/ocr',
      'wrapper' => 'ibook-url',
      'effect' => 'fade',
      'event' => 'change'),
  );
  $form['book_ahah_wrapper']['islandora_book_solr_subject_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr subject field'),
    '#description' => t('The solr field containing your untokenized subject value, used to link between books with the same subject.'),
    // keep default value as previous default so its backward compatible
    '#default_value' => variable_get('islandora_book_solr_subject_field', 'mods.subject'),
  );
  if ($ocr) {
    $form['book_ahah_wrapper']['islandora_ocr_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path to OCR executable'),
      '#description' => t('Path to OCR program on your server'),
      '#default_value' => $ocr_path,
      '#ahah' => array(
        'path' => 'islandora/book/ocr',
        'wrapper' => 'ibook-url',
        'effect' => 'fade',
        'event' => 'change'),
    );
    $form['book_ahah_wrapper']['infobox'] = array(
      '#type' => 'item',
      '#value' => $confirmation_message,
    );

    $form['book_ahah_wrapper']['refresh_page'] = array(
      '#type' => 'submit',
      '#value' => t('Test Path'),
      '#attributes' => array('class' => 'refresh-button'),
      '#submit' => array('book_admin_refresh'),
    );
  }

  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save Configuration'));
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults'));
  $form['#submit'][] = 'book_settings_form_submit';
  $form['#theme'] = 'system_settings_form';
  return ($form);
}

/**
 * update ocr div ??
 */
function update_ocr_div() {

  $form = book_callback_prep();
  $changed_elements = $form['book_ahah_wrapper'];
  unset($changed_elements['#prefix'], $changed_elements['#suffix']);
  $output = theme('status_messages') . drupal_render($changed_elements);
  drupal_json(array(
    'status' => TRUE,
    'data' => $output,
  ));
}

/**
 * book callback prep??
 * @return type 
 */
function book_callback_prep() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'rebuild' => TRUE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  return $form;
}

/**
 * book settings form submit ??
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function book_settings_form_submit($form, &$form_state) {

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($form_state['ahah_submission'] == TRUE) {
    $form_state['rebuild'] = TRUE;
    return;
  }
  if ($form_state['clicked_button']['#id'] != 'edit-submit' && $op != t('Reset to defaults')) {
    $form_state['rebuild'] = TRUE;
    return;
  }


  // Exclude unnecessary elements.
  unset($form_state['values']['submit'], $form_state['values']['reset'], $form_state['values']['form_id'], $form_state['values']['op'], $form_state['values']['form_token'], $form_state['values']['form_build_id']);

  foreach ($form_state['values'] as $key => $value) {
    if ($op == t('Reset to defaults')) {
      variable_del($key);
    }
    else {
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }
      variable_set($key, $value);
    }
  }
  if ($op == t('Reset to defaults')) {
    drupal_set_message(t('The configuration options have been reset to their default values.'));
  }
  else {
    drupal_set_message(t('The Islandora Book configuration options have been saved.'));
  }

  cache_clear_all();
  drupal_rebuild_theme_registry();
}

/**
 * book admin refresh
 * @param type $form
 * @param array $form_state 
 */
function book_admin_refresh($form, &$form_state) {
  $values = $form_state['values'];
  unset($form_state['submit_handlers']);
  form_execute_handlers('submit', $form, $form_state);
  $form_state['rebuild'] = TRUE;
}

function is_url_valid($url) {
  $handle = curl_init($url);
  curl_setopt($handle, CURLOPT_RETURNTRANSFER, TRUE);
  $response = curl_exec($handle);
  $httpCode = curl_getinfo($handle, CURLINFO_HTTP_CODE);
  $retval = TRUE;
  if ($httpCode % 100 == 4) {
    $retval = FALSE;
  }
  curl_close($handle);
  return $retval;
}

