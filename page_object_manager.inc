<?php

function page_management_wrapper($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  return drupal_render(get_page_model_management_content($pid));
}

function get_page_model_management_content($pid) {
  $form['manage_page_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Manage'),
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
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  global $base_url;


  $languages = islandora_book_get_tesseract_langauges();

  $edit_metadata_link = l(t('Edit Metadata'), $base_url . '/fedora/repository/editmetadata/' . $pid . '/metadata', array('attributes' => array('class' => 'islandora-repo-button')));

  $content_helper = new FedoraObjectDetailedContent($pid);
  $object_helper = new ObjectHelper();
  $ds_list = $object_helper->get_formatted_datastream_list($content_helper->pid, NULL, $content_helper->item);

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


  //$form['page_manage']['preprocess'] = array(
  //  '#title' => t('Preprocessing for typescripts?'),
  //  '#description' => t('Will add additional processing for typewritten text.  This script grayscales and sharpens an ingested .tiff before performing OCR processes. <br />
  //      This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive. <br />
  //      If your image is already gray, this script will not run'),
  //  '#type' => 'checkbox',
  //);


  $form['page_manage']['language'] = array(
    '#title' => t('Language'),
    '#type' => 'select',
    '#options' => $languages,
    '#selected' => 'eng',
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

  $form['renumber'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Renumber Page'),
  );
  $book_pids = get_collection_from_pid($pid);
  $page_number = get_page_number($pid);
  $book_pid = $book_pids[0];

  $page_count = count(get_sorted_pages($book_pid));
  $form['renumber']['new_page'] = array(
    '#type' => 'textfield',
    '#title' => t('New Sequence Number for page @num', array('@num' => $page_number)),
    '#size' => 5,
    '#description' => t('New sequence number must be between 1 and  @page_count inclusive', array('@page_count' => $page_count)),
  );
  $form['renumber']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Number'),
    '#id' => 'renumber_page',
  );

  if (user_access('delete pages')) {
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
  }

  return $form;
}

function page_management_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  global $user;
  $pid = $form_state['values']['pid'];
  $book_pids = get_collection_from_pid($pid);
  $current_page = get_page_number($pid);
  $book_pid = $book_pids[0];
  $pages = get_sorted_pages($book_pid);

  $item = new Fedora_Item($pid);

  if ($form_state['clicked_button']['#id'] == 'refresh_datastreams') {
    $tiff_file_path = save_datastream_as_tempfile($pid, 'TIFF');
    $language = $form_state['values']['language'];
    $preprocess = $form_state['values']['preprocess'];
    if ($form_state['values']['do_ocr']) {
      $success = refresh_ocr($tiff_file_path, $item, $language, $preprocess);
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
    $item->purge($pid . ' deleted by user identified as ' . $user->name);
    post_deletion_renumber_pages($book_pid);
    drupal_goto("fedora/repository/$book_pid");
  }

  if ($form_state['clicked_button']['#id'] == 'renumber_page') {
    $new_page = $form_state['values']['new_page'];
    $item = new Fedora_Item($pid);
    $item->purge_relationship('isPageNumber', $current_page, ISLANDORA_RELS_EXT_URI, TRUE);
    $item->purge_relationships('isSequenceNumber', $current_page, ISLANDORA_RELS_EXT_URI, TRUE);
    $item->add_relationship('isPageNumber', $new_page, ISLANDORA_RELS_EXT_URI, 3);
    $item->add_relationship('isSequenceNumber', $new_page, ISLANDORA_RELS_EXT_URI, 3);
    if ($new_page < $current_page) {
      for ($working_page = $new_page; $working_page < $current_page; $working_page++) {
        $item = new Fedora_Item($pages[$working_page]);
        //$item->purge_relationship('isPageNumber', sprintf("%04d", $working_page));
        //$item->add_relationship('isPageNumber', sprintf("%04d", $working_page + 1), ISLANDORA_PAGE_URI);
        $item->purge_relationships('isPageNumber', $working_page, ISLANDORA_RELS_EXT_URI, TRUE);
        $item->purge_relationships('isSequenceNumber', $working_page, ISLANDORA_RELS_EXT_URI, TRUE);
        $item->add_relationship('isPageNumber', $working_page + 1, ISLANDORA_RELS_EXT_URI, 3);
        $item->add_relationship('isSequenceNumber', $working_page + 1, ISLANDORA_RELS_EXT_URI, 3);
      }
    }

    if ($new_page > $current_page) {
      for ($working_page = $new_page; $working_page > $current_page; $working_page--) {
        $item = new Fedora_Item($pages[$working_page]);
        $item->purge_relationships('isPageNumber', $working_page, ISLANDORA_RELS_EXT_URI, TRUE);
        $item->purge_relationships('isSequenceNumber', $working_page, ISLANDORA_RELS_EXT_URI, TRUE);
        $item->add_relationship('isPageNumber', $working_page - 1, ISLANDORA_RELS_EXT_URI, 3);
        $item->add_relationship('isSequenceNumber', $working_page - 1, ISLANDORA_RELS_EXT_URI, 3);
      }
    }

    drupal_goto("fedora/repository/$pid");
  }
}

