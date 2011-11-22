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
    if (!$htmlstr){
      $htmlstr = '<div><div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/">View Full Size</a></div>' .
      '<div><a href="' . $base_url . '/fedora/repository/' . $this->pid . '/JPEG/"><img src="' .
      $base_url . '/fedora/repository/' . $this->pid . '/JPEG/' . '" /></a></div>';
    }
    $collection_fieldset = array(
      '#title' => 'Page View',
      '#attributes' => array(),
      '#collapsible' => FALSE,
      '#value' => $htmlstr,
    );
    return theme('fieldset', $collection_fieldset);
  }

}