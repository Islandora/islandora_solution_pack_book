<?php

/**
 * @file
 * Contains two local actions for ingesting pages.
 */

/**
 * Local menu action to present an ingest page form.
 *
 * @param AbstractObject $object
 *   The book to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function islandora_book_ingest_page(AbstractObject $object) {
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
    'objects' => array($page),
    'parent' => $object->id,
  ));
}
