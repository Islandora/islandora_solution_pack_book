<?php

function islandora_book_get_metadata_link(&$form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'plugins/FedoraObjectDetailedContent');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $edit_metadata_link = l(t('Edit Metadata'), $base_url . '/fedora/repository/editmetadata/' . $pid . '/metadata', array('attributes' => array('class' => 'islandora-repo-button')));
  $content_helper = new FedoraObjectDetailedContent($pid);
  $object_helper = new ObjectHelper();
  $dc_html = $object_helper->getFormattedDC($content_helper->item);

  $edit_metadata_link = l(t('Edit Metadata'), $base_url . '/fedora/repository/editmetadata/' . $pid . '/metadata', array('attributes' => array('class' => 'islandora-repo-button')));

  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to page view'), "fedora/repository/$pid"),
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