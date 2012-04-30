<?php

function view_metadata(&$form_state, $pid) {

  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'plugins/FedoraObjectDetailedContent');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
 
  global $base_url;

  $edit_metadata_link = l(t('Edit Metadata'), $base_url . '/fedora/repository/editmetadata/' . $pid . '/metadata', array('attributes' => array('class' => 'islandora-repo-button')));

  $content_helper = new FedoraObjectDetailedContent($pid);
  $objectHelper = new ObjectHelper();
  $dc_html = $objectHelper->getFormattedDC($content_helper->item);
  $form = array();
  $form['view_dc'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Metadata'),
  );


  $form['view_dc']['dublin_core'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $dc_html,
  );

  $form['view_dc']['edit'] = array(
    '#type' => 'markup',
    '#value' => $edit_metadata_link,
  );

  return $form;
}

