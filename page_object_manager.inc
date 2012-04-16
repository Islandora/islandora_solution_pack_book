<?php

function page_management_wrapper($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  return drupal_render(get_page_model_management_content($pid));
}

function get_page_model_management_content($pid) {
  $form['manage_page_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Manage This Page Object'),
    '#content' => drupal_get_form('page_management_form', $pid),
    '#selected' => FALSE,
  );
  return $form;
}

function page_management_form(&$form_state, $pid) {
  if (!user_access('manage page object')) {
    return;
  }
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'plugins/FedoraObjectDetailedContent');
  $content_helper = new FedoraObjectDetailedContent($pid);
  $object_helper = new ObjectHelper();
  $ds_list = datastream_display_builder($content_helper->pid, $content_helper->item);
  $add_datastreams_details = get_add_datastream_data($content_helper->pid);
  $dc_html = $object_helper->getFormattedDC($content_helper->item);
  $purge_form = drupal_get_form('fedora_repository_purge_object_form', $content_helper->pid, check_plain(substr(request_uri(), strlen(base_path()))));
  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );


  $form['view_dc'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('View Metadata'),
  );

  $form['view_dc']['dublin_core'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $dc_html,
  );
  $form['page_manage'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Update Derived Datastreams'),
    '#description' => t('Update datastreams for this page object.  The orginal stored tiff will be used to create all derived datastreams.'),
  );
  $form['page_manage']['do_ocr'] = array(
    '#type' => 'checkbox',
    '#title' => t('Refresh OCR for this page')
  );
  $form['page_manage']['recreate_derivatives'] = array(
    '#type' => 'checkbox',
    '#title' => t('Refresh all derived images')
  );

  $form['page_manage']['submit'] = array(
    '#type' => 'submit',
    '#id' => 'refresh_datastreams',
    '#value' => t('Update Page Datastreams'),
  );
  $form['current_datastreams'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Manage Current Datastreams'),
  );

  $form['current_datastreams']['current_object_details'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $ds_list,
  );
  $form['add_datastreams'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Add Additional Datastreams'),
  );
  if (count($add_datastreams_details) > 0) {
    $form['add_datastreams']['datastream_candidates'] = array(
      '#type' => 'tabpage',
      '#title' => t('View'),
      '#content' => $add_datastreams_details,
    );
  }
  if (user_access('delete pages')) {
    $form['delete_page'] = array(
      '#title' => "Permanently Delete this page",
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
  }

  return $form;
}

function page_management_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  global $user;
  $pid = $form_state['values']['pid'];
  $item = new Fedora_Item($pid);
  if ($form_state['clicked_button']['#id'] == 'refresh_datastreams') {
    $tiff_file_path = save_datastream_as_tempfile($pid, 'TIFF');
    if ($form_state['values']['do_ocr']) {
      $success = refresh_ocr($tiff_file_path, $item);
      if ($success) {
        drupal_set_message(t("OCR refreshed for this page"));
      }
    }
    if ($form_state['values']['recreate_derivatives']) {
      $success = refresh_images($tiff_file_path, $item);
      drupal_set_message("Refreshed $success of 3 image files for this page.");
    }
    file_delete($tiff_file_path);
  }
  if ($form_state['clicked_button']['#id'] == 'delete_page') {
    $book_pids = get_collection_from_pid($pid);
    $item->purge($pid . ' deleted by user identified as ' . $user->name);
    $book = $book_pids[0];
    drupal_goto("fedora/repository/$book");
  }
}

