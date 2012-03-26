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
    //   drupal_add_css('garbage.css');
    $htmlstr = $collectionHelper->getStream($this->pid, "IMT-HTML");
    $pstring = 'Image" src="' . $base_url . '/fedora/repository/' . $this->pid . '/IMT-JPG/';
    $htmlstr = preg_replace('/Image" src="([^"]+)"/', $pstring, $htmlstr);
    $htmlstr = preg_replace('/<\/body><\/html>/', '', $htmlstr);
    $htmlstr = preg_replace('/<\?xml(.*?)head>/s', '', $htmlstr);
    if (!$htmlstr) {
      $htmlstr = '<div><div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/">' . t('View Full Size') . '</a><br />' .
          ' <a href="' . $base_url . '/fedora/repository/' . $book . '">' . t('Return to Book View') . '</a></div>' .
          '<div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '"><img src="' .
          $base_url . '/fedora/repository/' . $this->pid . '/JPEG/' . '" /></a></div>';
    }
    $collection_fieldset = array(
      '#attributes' => array(),
      '#collapsible' => FALSE,
      '#value' => $htmlstr,
    );
    $tabset['pages_tab'] = array(
      '#type' => 'tabpage',
      '#title' => $item->objectProfile->objLabel,
      '#content' => theme('fieldset', $collection_fieldset),
    );
    return $tabset;
  }

}