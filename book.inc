<?php

/**
 * @file 
 * Islandora book 
 */

/**
 * Islandora Book
 */
class IslandoraBook {

  /**
   * Constructor
   * @param string $pid
   */
  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  /**
   * Show field sets
   * @global string $base_url
   * @global user $user
   * @return string 
   */
  public function showFieldSets() {
    module_load_include('inc', 'islandora_book', 'book_pack_utils');
    module_load_include('inc', 'islandora_book', 'add_pages');
    module_load_include('module', 'islandora_book');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    drupal_add_css(drupal_get_path('module', 'islandora_book') . '/css/islandora_book.css');
    global $base_url;
    $show_purge_tab = (!empty($_POST['form_id']) && ($_POST['form_id'] == 'islandora_book_purge_pages_form'));
    $page_number = 1;
    $pages_selected = FALSE;
    $qstring = $_GET['q'];
    $qparts = explode('/', $qstring);
    $tail = end($qparts);
    if (is_numeric($tail)) {
      $pages_selected = TRUE;
      $page_number = $tail;
    }
    $class = new CollectionClass();
    $results = $class->getRelatedItems($this->pid);
    $page_items = $class->renderCollection($results, $this->pid, NULL, NULL, $page_number);
    $page_pids = get_page_pids($this->pid);

    if (count($page_pids) > 0) {
      $hasPage = TRUE;
    }
    $tabset = array();

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }

    $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/islandora:viewerSdef/getViewer' . $qs;
    $html = '<iframe src="' . $viewer_url . '" scrolling="0" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';
    $tabset['pages_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Pages'),
      '#selected' => $pages_selected,
      '#content' => $page_items,
    );

    $tabset['first_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Description'),
      // This will be the content of the tab.
      '#content' => islandora_book_create_book_view($this->pid),
    );

    if ($hasPage) {
      $tabset['second_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Read'),
        '#content' => $html
      );
    }

    if (!$hasPage && user_access('ingest new fedora objects')) {
      $tabset['third_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add pages'),
        '#content' => drupal_get_form('book_add_pages_form', $this->pid),
      );
    }
    if ($hasPage && user_access('purge objects and datastreams')) {
      $tabset['fourth_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Purge all pages'),
        '#selected' => $show_purge_tab,
        '#content' => drupal_get_form('islandora_book_purge_pages_form', $this->pid),
      );
    }
    return $tabset;
  }

}

