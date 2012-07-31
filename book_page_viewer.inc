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

    $link = array();
    $links[] = l(t('Return to Book View'), "fedora/repository/$book");
    $links = array_merge($links, module_invoke_all('islandora_book_get_pageview_links', $this->pid));
    $htmlstr = theme_item_list($links);
    $htmlstr .= '<div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/"><img src="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/' . '" /></a></div>';

    $collection_fieldset = array(
      '#attributes' => array(),
      '#collapsible' => FALSE,
      '#value' => $htmlstr,
    );
    $tabset['pages_tab'] = array(
      '#type' => 'tabpage',
      '#title' => $item->objectProfile->objLabel,
      '#content' => theme('fieldset', $collection_fieldset),
      '#selected' => true,
    );
    return $tabset;
  }

}
