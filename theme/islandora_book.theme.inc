<?php

/**
 * @file
 *
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_book(array &$variables) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $variables['object'];
  $params = array(
    'object' => $object,
    'pages' => array_keys(islandora_book_get_pages($object)),
    'page_progression' => islandora_book_get_page_progression($object),
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
  $variables['book_object_id'] = $results[0]['object']['value'];
  $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view", array('absolute' => TRUE));
  $viewer = islandora_get_viewer($jp2_url, 'islandora_book_page_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_pages(array &$variables) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $variables['object'];
  // View Links
  $display = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $grid_active = ($display == 'grid') ? 'active' : '';
  $list_active = ($display == 'active') ? 'active' : '';
  $query_params = drupal_get_query_parameters($_GET);
  $variables['view_links'] = array(
    array(
      'title' => 'Grid view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-grid $grid_active"
      ),
      'query' => $query_params + array('display' => 'grid')
    ),
    array(
      'title' => 'List view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-list $list_active"
      ),
      'query' => $query_params + array('display' => 'list')
    )
  );
  // Pager
  $pages = islandora_book_get_pages($object);
  $page_size = (empty($_GET['pagesize'])) ? 20 : $_GET['pagesize'];
  $total_count = count($pages);
  pager_default_initialize($total_count, $page_size);
  $variables['pager'] = theme('pager', array('quantity' => 10));
  // Content
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $start = $page_size * ($page_number);
  $end = min($start + $page_size, $total_count);
  // Only display current page.
  $pages = array_splice($pages, $start, $end);
  $pages = array_map(function($o) {
             $object_url = "islandora/object/{$o['pid']}";
             $img = theme_image(array('path' => url("$object_url/datastream/TN/view"), 'attributes' => array()));
             $dc = DublinCore::import_from_xml_string(islandora_datastream_load('DC', $o['pid'])->content);
             return $o + array(
               'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $o['pid'])),
               'link' => l($o['label'], $object_url, array('html' => TRUE, 'attributes' => array('title' => $o['label']))),
               'image' => l($img, $object_url, array('html' => TRUE, 'attributes' => array('title' => $o['label']))),
               'DC' => $dc->as_formatted_array()
             );
           }, $pages);
  $theme = ($display == 'grid') ? 'islandora_book_pages_grid' : 'islandora_book_pages_list';
  $variables['content'] = theme($theme, array('object' => $object, 'pages' => $pages));
  $module_path = drupal_get_path('module', 'islandora_book');
  drupal_add_css("$module_path/css/islandora_book.css");
}

/**
 * Implements theme_form_id().
 */
function theme_islandora_book_manage_book_sequences_form($variables) {
  $form = $variables['form'];
  $rows = array();
  $weight_class = 'islandora-book-page-weight';
  $table = &$form['table'];
  foreach (element_children($table['rows']) as $id) {
    $row = &$table['rows'][$id];
    $row['weight']['#attributes'] = array('class' => array($weight_class));
    $rows[] = array(
      'data' => array(
        drupal_render($row['pid']),
        drupal_render($row['label']),
        drupal_render($row['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $table_id = 'islandora-reorder-pages-table';
  $output = theme('table', array('header' => $table['#header'], 'rows' => $rows, 'attributes' => array('id' => $table_id)));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag($table_id, 'order', 'sibling', $weight_class);
  return $output;
}
