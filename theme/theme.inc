<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_book(array &$variables) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $object = $variables['object'];
  $params = array(
    'object' => $object,
    'pages' => islandora_paged_content_get_pages($object),
    'page_progression' => islandora_paged_content_get_page_progression($object),
  );
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $viewer = islandora_get_viewer($params, 'islandora_book_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_page(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $object = $variables['object'];
  $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $result = reset($results);
  $variables['book_object_id'] = $result ? $result['object']['value'] : FALSE;

  $params = array();
  if (isset($object['JP2']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JP2'])) {
    // Get token to allow access to XACML protected datastreams.
    // Always use token authentication in case there is a global policy.
    module_load_include('inc', 'islandora', 'includes/authtokens');
    $token = islandora_get_object_token($object->id, 'JP2', 2);
    $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view", array(
      'absolute' => TRUE,
      'query' => array('token' => $token),
    ));
    $params['jp2_url'] = $jp2_url;
  }

  $viewer = islandora_get_viewer($params, 'islandora_book_page_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
  }
}
