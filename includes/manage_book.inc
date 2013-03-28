<?php

/**
 * @file
 * Contains one local action for ingesting pages.
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
  $tuque = islandora_get_tuque_connection();
  $page = $tuque->repository->constructObject(islandora_get_namespace($issue->id));
  $page->owner = $user->name;
  $page->label = 'New Page';
  $page->models = 'islandora:pageCModel';
  drupal_set_title(t('Add page to @book', array('@book' => $object->label)));
  return drupal_get_form('islandora_ingest_form', array(
      'book' => $object->id,
      'models' => array('islandora:pageCModel'),
      'object' => $page,
    )
  );
}
