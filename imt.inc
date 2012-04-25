<?php

class IMTViewer {

  private $pid = NULL;

  /**
   * Constructor
   * @param type $pid
   */
  function __construct($pid) {
    $this->pid = $pid;
  }

  function showIMT() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'islandora_book', 'book_pack_utils');
    // hook added to allow support modules to additional links to page.
    $links = module_invoke_all('islandora_book_get_pageview_links', $this->pid);
    $books = get_collection_from_pid($this->pid);
    $book = $books[0];
    $item = new fedora_item($this->pid);
    $path = drupal_get_path('module', 'islandora_book');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $collectionHelper = new CollectionClass();
    $pathTojs = $path . '/js/imt_template.js';
    drupal_add_js("$pathTojs");
    $pathToCSS = $path . '/js/imt_template.css';
    drupal_add_css("$pathToCSS");

    $options = array(
      'attributes' => array('target' => '_blank'),
      'query' => array(
        'PID' => $this->pid,
      ),
    );
    $viewTag = l(t('View Full Size'), 'fedora/repository/' . $this->pid . '/JPEG/');
    $returnTag = l(t('Return to Book View'), "fedora/repository/$book");


    $htmlstr = "<div>$viewTag</div>";
    $htmlstr .= "<div>$returnTag</div>";
    foreach($links as $link){
      $htmlstr .= "<div>$link</div>";
    }
    $htmlstr .= '<div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '"><img src="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/' . '" /></a></div>';


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
