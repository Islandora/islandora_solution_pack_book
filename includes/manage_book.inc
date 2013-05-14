<?php

/**
 * @file
 * Contains two local actions for ingesting pages.
 */

/**
 * Local menu action to present an ingest page form.
 *
 * @param FedoraObject $object
 *   The book to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function islandora_book_ingest_page(FedoraObject $object) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $tuque = islandora_get_tuque_connection();
  $page = $tuque->repository->constructObject(islandora_get_namespace($object->id));
  $page->owner = $user->name;
  $page->label = 'New Page';
  $page->models = 'islandora:pageCModel';
  drupal_set_title(t('Add page to @book', array('@book' => $object->label)));
  return drupal_get_form('islandora_ingest_form', array(
    'book' => $object,
    'models' => array('islandora:pageCModel'),
    'object' => $page,
    'parent' => $object->id,
      )
  );
}

/**
 * Local menu action to present a zipped file ingest form.
 *
 * @param FedoraObject $object
 *   The book to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function islandora_book_ingest_zipped_pages(FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  return drupal_get_form('islandora_book_zipped_upload_form', $object->id);
}

/**
 * Defines the zipped page form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $book_pid
 *   PID of book into which pages are being ingested.
 *
 * @return array
 *   Drupal form definition.
 */
function islandora_book_zipped_upload_form(array $form, array &$form_state, $book_pid) {
  module_load_include('inc', 'islandora_ocr', 'includes/utilities');
  $languages = module_exists('islandora_ocr') ? islandora_ocr_get_enabled_tesseract_languages() : array();
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('zip');
  $form = array();
  $derivatives = variable_get('islandora_book_ingest_derivatives', array('ocr'));
  $do_ocr = in_array('ocr', $derivatives);
  $form['language'] = array(
    '#access' => module_exists('islandora_ocr') && $do_ocr,
    '#title' => t('Language'),
    '#type' => 'select',
    '#description' => t('Please select the language the page is written in.'),
    '#options' => $languages,
    '#default_value' => 'English',
  );

  $form['file'] = array(
    '#title' => t('Compressed image file.'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['book_pid'] = array(
    '#type' => 'hidden',
    '#value' => $book_pid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add files to book'),
  );
  return $form;
}


/**
 * Submit handler for uploaded zip files.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_zipped_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $book_pid = $form_state['values']['book_pid'];
  $namespace = substr($book_pid, 0, strpos($book_pid, ":"));
  $tmp_dir = uniqid();
  $book_object = islandora_object_load($book_pid);
  if (!$book_object) {
    drupal_set_message(t("This book does not exist in this repository"), 'warning');
    return;
  }
  $current_pages = islandora_paged_content_get_pages($book_object);
  $last_page_number = count($current_pages);
  // Extract file.
  $zip_file = file_load($form_state['values']['file']);
  $zip_uri = drupal_realpath($zip_file->uri);
  $zip = new ZipArchive();
  $zip->open($zip_uri);
  $destination_dir = drupal_realpath("temporary://$tmp_dir");

  // Extract zipped file to named directory.
  if (!$zip->extractTo($destination_dir)) {
    drupal_set_message(t('Ingest failed.'), 'warning');
    return;
  }
  $zip->close();
  $ignore = array('.', '..', '__MACOSX');
  $callback = function($element) use ($ignore) {
    return (!in_array($element, $ignore)) ? TRUE : FALSE;
  };
  $unfiltered = scandir($destination_dir);
  $files_to_add = array_values(array_filter($unfiltered, $callback));
  $batch = array(
    'title' => t("Adding new pages to book."),
    'progress_message' => t('Added @current pages out of @total.'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/manage_book.inc',
  );
  $config = array(
    'book_pid' => $book_pid,
    'destination_dir' => $destination_dir,
    'namespace' => $namespace,
    'language' => $form_state['values']['language'],
  );
  foreach ($files_to_add as $image) {
    $config['page_number'] = ++$last_page_number;
    $config['image'] = $image;
    $batch['operations'][] = array(
      'islandora_book_add_pages',
      array($repository, $config),
    );
  }
  batch_set($batch);
  batch_process();
}

/**
 * Creates page objects and associates them with book object.
 *
 * @param Repository $repository
 *   Active repository object to build NewFedoraObject
 * @param array $config
 *   Associative array of required values.
 */
function islandora_book_add_pages($repository, $config) {
  module_load_include('inc', 'islandora', 'includes/IslandoraTuque');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  // Create object.
  $object = $repository->constructObject($config['namespace']);
  $object->label = $config['image'];
  $file_location = $config['destination_dir'] . '/' . $config['image'];
  islandora_paged_content_update_datastream($object, $file_location, 'OBJ', NULL, NULL, 'M', FALSE);
  $rels_ext = $object->relationships;
  $parent = $config['book_pid'];
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:pageCModel');
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $parent);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $config['page_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $config['page_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $parent);
  islandora_add_object($object);
}
