<?php

function islandora_book_display_links($function, $pid) {
  $mappings = array(
    'pages' => 'get_page_display'
  );
  return drupal_get_form($mappings[$function], $pid);
}

function get_page_display(&$form_state, $pid) {

  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $class = new CollectionClass();
  $query = <<<XML
select \$object \$title  from <#ri>
where \$object <fedora-rels-ext:isMemberOf> <info:fedora/$pid>
and \$object <fedora-model:state> <fedora-model:Active>
and (\$object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> \$title
or \$object <http://islandora.ca/ontology/relsext#isSequenceNumber> \$title)
order by \$title
XML;
  $results = $class->getRelatedItems($pid, $query);
  $page_items = $class->renderCollection($results, $pid, NULL, NULL, $page_number);
  $form = array();
  $form['link'] = array(
    '#type' => 'item',
    '#value' => l(t('Return to book view'), "fedora/repository/$pid"),
  );
  $form['pages_tab'] = array(
    // #type and #title are the minimum requirements.
    '#type' => 'tabpage',
    '#title' => t('Pages'),
    '#content' => $page_items,
    '#weight' => 15,
  );
  return $form;
}