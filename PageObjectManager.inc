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
  module_load_include('inc', 'fedora_repository', 'plugins/FedoraObjectDetailedContent');
  $content_helper = new FedoraObjectDetailedContent($pid);
  $objectHelper = new ObjectHelper();
  $ds_list = display_builder($content_helper->pid, $content_helper->item);
  $add_datastreams_details = get_add_datastream_data($content_helper->pid);
  $dc_html = $objectHelper->getFormattedDC($content_helper->item);
  $purge_form = drupal_get_form('fedora_repository_purge_object_form', $content_helper->pid, check_plain(substr(request_uri(), strlen(base_path()))));
  $form = array();
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );


  $form['view_dc'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('View Metadata'),
  );

  $form['view_dc']['dublin_core'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $dc_html,
  );
  $form['page_manage'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
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
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Manage Current Datastreams'),
  );

  $form['current_datastreams']['current_object_details'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $ds_list,
  );
  $form['add_datastreams'] = array(
    '#type' => 'fieldset',
    '#collapsed' => true,
    '#collapsible' => true,
    '#title' => t('Add Additional Datastreams'),
  );

  $form['add_datastreams']['datastream_candidates'] = array(
    '#type' => 'tabpage',
    '#title' => t('View'),
    '#content' => $add_datastreams_details,
  );
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
  global $user;
  $pid = $form_state['values']['pid'];
  $tmp_filename = preg_replace("/:/", '_', $pid) . 'tif';
  $item = new Fedora_Item($pid);
  if ($form_state['clicked_button']['#id'] == 'refresh_datastreams') {
    $tiff = do_curl("http://localhost:8080/fedora/objects/$pid/datastreams/TIFF/content");
    $tiff_file_path = file_directory_path() . "/" . $tmp_filename;
    file_put_contents($tiff_file_path, $tiff);
    $item = new Fedora_Item($pid);
    if ($form_state['values']['do_ocr']) {
      refresh_ocr($tiff_file_path, $item);
    }
    if ($form_state['values']['recreate_derivatives']) {
      refresh_images($tiff_file_path, $item);
    }
    file_delete($tiff_file_path);
  }
  if ($form_state['clicked_button']['#id'] == 'delete_page') {
    $item->purge($pid . ' deleted by user identified as ' . $user->name);
  }
}

function refresh_ocr($tiff, &$item) {
  $label = $item->objectProfile->objLabel;
  $datastreams = $item->get_datastreams_list_as_array();
  $executable = variable_get('islandora_ocr_path', '/usr/bin/tesseract');
  $returnValue = "";
  $output = "";
  exec("$executable $tiff $tiff -l eng", $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('OCR', $datastreams)) {
      $item->purge_datastream('OCR');
    }
    $item->add_datastream_from_file("$tiff.txt", 'OCR', "Scanned Text", 'text/plain', 'M');
    file_delete("$tiff.txt");
  }

  drupal_set_message("Updated OCR for $label");
}

function refresh_images($tiff, &$item) {
  $label = $item->objectProfile->objLabel;
  $datastreams = $item->get_datastreams_list_as_array();
  $returnValue = "";
  exec('kdu_compress -i "' . $tiff . '" -o "' . $tiff . '.jp2' . '" -rate 0.5 Clayers=1 Clevels=7 "Cprecincts={256,256},{256,256},{256,256},{128,128},{128,128},{64,64},{64,64},{32,32},{16,16}" "Corder=RPCL" "ORGgen_plt=yes" "ORGtparts=R" "Cblk={32,32}" Cuse_sop=yes', $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('JP2', $datastreams)) {
      $item->purge_datastream('JP2');
    }
    $item->add_datastream_from_file("$tiff.jp2", 'JP2', "Compressed JP2", 'image/jp2', 'M');
    file_delete("$tiff.jp2");
  }

  exec("convert {$tiff}[0] -colorspace RGB -thumbnail 120x $tiff-tn.jpg", $output, $returnValue);

  if ($returnValue == '0') {
    if (array_key_exists('TN', $datastreams)) {
      $item->purge_datastream('TN');
    }
    $item->add_datastream_from_file("$tiff-tn.jpg", 'TN', 'Thumbnail', 'image/jpeg', 'M');
    file_delete("$tiff-tn.jpg");
  }
  file_delete("$tiff-tn.jpg");
  exec("convert  -resize 800 \"{$tiff}\"[0]  \"$tiff-med.jpg\"", $output, $returnValue);
  if ($returnValue == '0') {
    if (array_key_exists('JPEG', $datastreams)) {
      $item->purge_datastream('JPEG');
    }
    $item->add_datastream_from_file("$tiff-med.jpg", 'JPEG', "JPEG image", 'image/jpeg', 'M');
    file_delete("$tiff-med.jpg");
  }
  drupal_set_message("Updated Derivative Images for $label");
}

function display_builder($object_pid, &$fedoraItem) {
  global $fedoraUser, $fedoraPass, $base_url, $user;
  module_load_include('inc', 'fedora_repository', 'ConnectionHelper');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'ContentModel');
  $object_helper = new ObjectHelper();
  $path = drupal_get_path('module', 'fedora_repository');
  $dataStreamBody = '';
  $fedoraItem = new Fedora_Item($object_pid);

  if (user_access(ObjectHelper :: $VIEW_DETAILED_CONTENT_LIST)) {
    $current_datastreams = 'All Datastreams';
    //$metaDataText='Description';
    $mainStreamLabel = NULL;
    $object = $fedoraItem->get_datastreams_list_as_SimpleXML();
    if (!isset($object)) {
      drupal_set_message(t("No datastreams available"));
      return ' ';
    }
    $hasOBJStream = NULL;
    $hasTNStream = FALSE;
    $dataStreamBody = "<br /><table>\n";

    $cmDatastreams = array();
    if (variable_get('fedora_object_restrict_datastreams', FALSE) == TRUE && ($cm = ContentModel::loadFromObject($object_pid)) !== FALSE) {
      $cmDatastreams = $cm->listDatastreams();
    }

    $dataStreamBody .= $object_helper->get_parent_objects_asHTML($object_pid);
    $dataStreamBody .= '<tr><th colspan="4"><h3>' . t("!text", array('!text' => $current_datastreams)) . '</h3></th></tr>';
    foreach ($object as $datastream) {
      foreach ($datastream as $datastreamValue) {
        if (variable_get('fedora_object_restrict_datastreams', FALSE) == FALSE || ((isset($user) && in_array('administrator', $user->roles)) || in_array($datastreamValue->ID, $cmDatastreams))) {
          if ($datastreamValue->ID == 'OBJ') {
            $hasOBJStream = '1';
            $mainStreamLabel = $datastreamValue->label;
            $mainStreamLabel = str_replace("_", " ", $mainStreamLabel);
          }
          if ($datastreamValue->ID == 'TN') {
            $hasTNStream = TRUE;
          }
          //create the links to each datastream
          $dataStreamBody .= $object_helper->create_link_for_ds($object_pid, $datastreamValue); //"<tr><td><b>$key :</b></td><td>$value</td></tr>\n";
        }
      }
    }
    $dataStreamBody .= "</table>\n";
    return $dataStreamBody;



    //if they have access let them add a datastream
    if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
      $allow = TRUE;
      if (module_exists('fedora_fesl')) {
        $allow = fedora_fesl_check_roles($object_pid, 'write');
      }
      if ($allow) {
        $dataStreamBody .= drupal_get_form('add_stream_form', $object_pid);
      }
    }
    $fieldset = array(
      '#title' => t("!text", array('!text' => $availableDataStreamsText)),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#value' => $dataStreamBody
    );
    $dataStreamBody = '<div class = "fedora_detailed_list">' . theme('fieldset', $fieldset) . '</div>';

    return $dataStreamBody;
  }
  return '';
}

function get_add_datastream_data($object_pid) {
  if (user_access(ObjectHelper :: $ADD_FEDORA_STREAMS)) {
    $allow = TRUE;
    if (module_exists('fedora_fesl')) {
      $allow = fedora_fesl_check_roles($object_pid, 'write');
    }
    if ($allow) {
      $dataStreamBody .= drupal_get_form('add_stream_form', $object_pid);
    }
  }
  return $dataStreamBody;
}