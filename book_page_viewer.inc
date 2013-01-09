<?php

class PageViewer {

  private $pid = NULL;

  /**
   * Constructor
   * @param type $pid
   */
  function __construct($pid) {
    $this->pid = $pid;
  }

  function showPage() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'islandora_book', 'book_pack_utils');
    $books = get_collection_from_pid($this->pid);
    $book = $books[0];
    $item = new fedora_item($this->pid);
    $pid = $this->pid;
    $links = array();
    $links[] = l(t('Return to Book View'), "fedora/repository/$book");
    $links = array_merge($links, module_invoke_all('islandora_book_get_pageview_links', $pid));
    $htmlstr = theme_item_list($links);
    $alink_options = array(
      'attributes' => array('target' => '_blank', 'class' => 'fedora_image', 'title' => $pid),
      'html' => TRUE,
    );

    $img = theme_image("$base_url/fedora/repository/$pid/JPEG", '', NULL, NULL, FALSE);
    $htmlstr .= l($img, 'fedora/repository/'. $this->pid . '/JPEG/',$alink_options);

    $tabset['pages_tab'] = array(
      '#type' => 'tabpage',
      '#title' => $item->objectProfile->objLabel,
      '#content' => $htmlstr,
      '#selected' => true,
    );
    return $tabset;
  }

}
