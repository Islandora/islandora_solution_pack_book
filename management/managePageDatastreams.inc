<?php

function islandora_book_page_datastreams($form, $pid) {
  module_load_include('inc', 'fedora_repository', 'plugins/FedoraObjectDetailedContent');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $content_helper = new FedoraObjectDetailedContent($pid);
  $object_helper = new ObjectHelper();
  $ds_list = $object_helper->get_formatted_datastream_list($content_helper->pid, NULL, $content_helper->item);
  $form = array();
  $ds_list = $object_helper->get_formatted_datastream_list($content_helper->pid, NULL, $content_helper->item);

  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to page view'), "fedora/repository/$pid"),
  );
  $form['current_object_details'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $ds_list,
  );
  return $form;
}

function islandora_book_page_delete($form, $pid) {
  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to page view'), "fedora/repository/$pid"),
  );
  $form['delete_page'] = array(
    '#title' => t("Permanently Delete This Page"),
    '#type' => 'fieldset',
    '#description' => t("Clicking this button will delete this page. <br /> <strong>This action cannot be undone.</strong>"),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['delete_page']['confirm'] = array(
    '#title' => "Are you sure?",
    '#type' => 'fieldset',
    '#description' => t('<strong>Clicking the delete button will permanantly remove this page. <br />  <strong>This action cannot be undone.</strong> '),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['delete_page']['confirm']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete this page'),
    '#id' => 'delete_page',
  );
  return $form;
}

function islandora_book_page_delete_submit($form, &$form_state) {
  global $user;
  $item->purge($pid . ' deleted by user identified as ' . $user->name);
  post_deletion_renumber_pages($book_pid);
  drupal_goto("fedora/repository/$book_pid");
}