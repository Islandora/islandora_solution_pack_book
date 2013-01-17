<?php

/*
 * This file should be put in the islandora modules plugins directory
 * typical usage: drush -u 1 islandora_update_book_rels path_to_query_file TRUE
 * the above would give you an interactive update for each book.  If TRUE is changed to FALSE it will run against all
 * books returned from the query

 * 
 * If this is being run from a multi site env. you will probably have to use the --uri switch
 * 'drush -u 1 --uri=http://localhost islandora_update_book_rels /path/to/query.txt TRUE'.  You should also be in the subsites
 * sites directory or a subdirectory of it.
 * 
 */

//drush hook

function islandora_update_book_rels_drush_command() {
  $items = array();

  $items['islandora_update_book_rels'] = array(
    'description' => "updates books and pages so the RELS-EXT datastream is compliant with the Islandora internet Archive book viewer",
    'arguments' => array(
      'query_file' => 'The path to a plain text file that contains an itql query.  This query should return a list of books (pids)',
      'interactive' => 'if TRUE then you will be asked to confirm the update for each book object'
    ),
    'examples' => array(
      'drush -u 1 --uri=http://localhost islandora_update_book_rels /path/to/query.txt TRUE',
    ),
    'aliases' => array('islandoraubr'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
  );
  $items['islandora-remove-rels-statements'] = array(
    'description' => "updates book pages, removes some statements from the rels (THIS IS FOR TESTING PURPOSES ONLY).  We remove rels statements with this script so we can add them back via update_rels script",
    'arguments' => array(
      'query_file' => 'The path to a plain text file that contains an itql query.  This query should return a list of books (pids)',
      'interactive' => 'if TRUE then you will be asked to confirm the update for each book object'
    ),
    'examples' => array(
      'drush -u 1 --uri=http://localhost islandora_remove_rels_statements /path/to/query.txt TRUE',
    ),
    'callback' => 'drush_islandora_remove_rels_statements',
    //'callback arguments' => array(dirname(__FILE__) . '/sandwich-topic.txt'),
    'aliases' => array('islandoraremoverels'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
  );

  return $items;
}

/**
 * This is the drush command specified in the array create by the drush entry point.

 * 
 * This function checks to make sure parameters are supplied and if everything is ok
 * calls the doAction function and adds the delete parameter
 * 
 * @param string $query_file
 *  path to a text file that contains an itql query
 * @param boolean $interactive
 * 
 */
function drush_islandora_remove_rels_statements($query_file, $interactive) {
  drush_print('Current working directory ' . getcwd());
  if (isset($query_file)) {
    drush_print(" using query file" . $query_file);
  }
  else {
    drush_print(" no query_file found");
    return;
  }
  islandora_update_book_rels_doAction($query_file, $interactive, 'DELETE');
}

/**
 * This is the drush command specified in the array create by the drush entry point.

 * 
 * This function checks to make sure parameters are supplied and if everything is ok
 * calls the doAction function
 * 
 * @param string $query_file
 *  path to a text file that contains an itql query
 * @param boolean $interactive
 * 
 */
function drush_islandora_update_book_rels($query_file, $interactive) {
  drush_print('Current working directory ' . getcwd());
  if (isset($query_file)) {
    drush_print(" using query file" . $query_file);
  }
  else {
    drush_print(" no query_file found");
    return;
  }
  islandora_update_book_rels_doAction($query_file, $interactive);
}

/**
 * Iterates through all the books returned by the itql query and updates the rels stream for the pages of each book

 * 
 * @param string $query_file
 * @param boolean $interactive
 * 
 */

function islandora_update_book_rels_doAction($query_file, $interactive, $action = 'UPDATE') {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $query = file_get_contents($query_file);
  drush_print($query);
  if (!isset($query)) {
    drush_die('no query found');
    return;
  }
  $results = islandora_run_query($query);
  $objects = islandora_sparql_results_as_array($results);
  foreach ($objects as $object) {
    drush_print($object);
  }
  $num = count($objects);

  if (!drush_confirm(dt('are you sure you want to %s all page relationships in @num of books?', array('@num' => $num, '%s' => $action)))) {
// was: return drush_set_error('CORE_SYNC_ABORT', 'Aborting.');
    drush_die('Aborting.');
  }

  if ($interactive == 'TRUE') {
    foreach ($objects as $object) {
      if (drush_confirm(dt('%s all page relationships in this book @pid ?', array('@pid' => $object, '%s' => $action)))) {
//$item = new Fedora_Item($object);

        islandora_drush_update_book_rels_stream($object, $action);
      }
    }
  }
  else {
    foreach ($objects as $object) {
//$item = new Fedora_Item($object);

      islandora_drush_update_book_rels_stream($object, $action);
    }
  }
}

/**

 * returns an array of pids 
 * @todo pull this up into an api
 * @param SimpleXMLElement $content
 * @return array
 */
function islandora_sparql_results_as_array($content) {
  $content = new SimpleXMLElement($content);

  $resultsarray = array();
  foreach ($content->results->result as $result) {
    $resultsarray[] = substr($result->object->attributes()->uri, 12); // Remove 'info:fedora/'.
  }
  return $resultsarray;
}

/**
 * update all the pages objects rels-ext of this book
 * TODO refactor so add_relationship and purge_relationship become their own functions called from this function
 * @param string $book_pid
 * 
 */
function islandora_drush_update_book_rels_stream($book_pid, $action = 'UPDATE') {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');


  $page_pids = islandora_update_book_rels_get_page_pids($book_pid);
  $page_num = 1;
  $namespaces = array(RELS_EXT_URI, FEDORA_MODEL_URI, ISLANDORA_PAGE_URI, ISLANDORA_RELS_EXT_URI, ISLANDORA_RELS_INT_URI);
//update all the page rels
  foreach ($page_pids as $pid) {
    $item = new Fedora_Item($pid);
    if ($item->exists()) {
      $relationships = islandora_update_book_get_relationships($item, $namespaces);
      if ($action != 'DELETE') {
        if (!array_key_exists('isPageNumber', $relationships)) {
          $item->add_relationship('isPageNumber', $page_num, ISLANDORA_RELS_EXT_URI, 3);
          drush_print(dt('%s page number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('page number already exists for @pid', array('@pid' => $pid)));
        }
        if (!array_key_exists('isSequenceNumber', $relationships)) {
          $item->add_relationship('isSequenceNumber', $page_num++, ISLANDORA_RELS_EXT_URI, 3);
          drush_print(dt('%s sequence number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('sequence number already exists for @pid', array('@pid' => $pid)));
        }
        if (!array_key_exists('isSection', $relationships)) {
          $item->add_relationship('isSection', '1', ISLANDORA_RELS_EXT_URI, 3);
          drush_print(dt('%s section number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('section already exists for @pid', array('@pid' => $pid)));
        }
        if (!array_key_exists('isPageOf', $relationships)) {
          $item->add_relationship('isPageOf', $book_pid, ISLANDORA_RELS_EXT_URI);
          drush_print(dt('updated page of for @pid', array('@pid' => $pid)));
        }
        else {
          drush_print(dt('is pageof already exists for @pid', array('@pid' => $pid)));
        }
        //we want to remove relationships here
      }
      else {
        if (array_key_exists('isPageNumber', $relationships)) {
          drush_print(dt($item->purge_relationships('isPageNumber', $page_num, ISLANDORA_RELS_EXT_URI, TRUE)));
          drush_print(dt('%s page number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('relationship did not exist for @pid', array('@pid' => $pid)));
        }
        if (array_key_exists('isSequenceNumber', $relationships)) {
          drush_print(dt($item->purge_relationships('isSequenceNumber', $page_num++, ISLANDORA_RELS_EXT_URI, TRUE)));
          drush_print(dt('%s sequence number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('sequence number did not exist so did not update @pid', array('@pid' => $pid)));
        }
        if (array_key_exists('isSection', $relationships)) {
          drush_print(dt($item->purge_relationships('isSection', '1', ISLANDORA_RELS_EXT_URI, TRUE)));
          drush_print(dt('%s section number for @pid', array('@pid' => $pid, '%s' => $action)));
        }
        else {
          drush_print(dt('section did not exist so did not update for @pid', array('@pid' => $pid)));
        }


      }
      $item->forget();
    }



//islandora_update_book_create_rels($book_pid, $pid, $page_num++);//used by UPEI only will overwrite existing rels

    time_nanosleep(0, 200000000); //give the server some rest between calls
  }
}

/**
 * TODO: update the current fedoraItem getrelationship with this function
 *
 * Currently the Fedora API call getRelationships is reporting an uncaught
 * exception so we will parse the RELS-EXT ourselves and simulate the
 * documented behaviour.

 * @param Fedora_Item $item 
 * @param array $namespaces
 *   an array of namespaces used to match elements
 */
function islandora_update_book_get_relationships(&$item, $namespaces = array(RELS_EXT_URI, FEDORA_MODEL_URI, ISLANDORA_PAGE_URI)) {
  $relationships = array();
  try {
    $relsext = $item->get_datastream_dissemination('RELS-EXT');
  } catch (exception $e) {

    return $relationships;
  }

// Parse the RELS-EXT into an associative array.
  $relsextxml = new DOMDocument();
  $relsextxml->loadXML($relsext);
  $relsextxml->normalizeDocument();
  $allTags = array();
  foreach ($namespaces as $namespace) {
    $allTags[] = $relsextxml->getElementsByTagNameNS($namespace, '*');
  }

  foreach ($allTags as $tags) {
    foreach ($tags as $child) {
      $value = preg_replace('/info:fedora\//', '', $child->getAttributeNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'resource'));
      if (empty($value)) {
//value is probably not a resource lets check for a literal
        $value = (string) $child->nodeValue;
      }
      $relationships[$child->tagName][] = $value;
    }
  }

  return $relationships;
}

/**
 * this function used by UPEI when we want to minimize jms events.  we update the relationships
 * in one call to modify datastream instead of many to add relationship. This shouldn't be used
 * in general use cases as users may have added other rels statements and this will wipe out the other
 * statements if they exist.
 * @param string $book_pid
 * @param string $pid
 * @param string $page_number 
 */
function islandora_update_book_create_rels($book_pid, $pid, $page_number) {
  $rdf_string = <<<RDF
  <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
<rdf:Description rdf:about="info:fedora/$pid">
	<hasModel xmlns="info:fedora/fedora-system:def/model#" rdf:resource="info:fedora/islandora:pageCModel"></hasModel>
	<isSequenceNumber xmlns="http://islandora.ca/ontology/relsext#">$page_number</isSequenceNumber>
	<isMemberOf xmlns="info:fedora/fedora-system:def/relations-external#" rdf:resource="info:fedora/$book_pid"></isMemberOf>
	<isSection xmlns="http://islandora.ca/ontology/relsext#">1</isSection>
	<isPageOf xmlns="http://islandora.ca/ontology/relsext#" rdf:resource="inf:fedora/$book_pid"></isPageOf>
  <isPageNumber xmlns="http://islandora.ca/ontology/relsext#">$page_number</isPageNumber>
</rdf:Description>
</rdf:RDF>
RDF;
//drush_print($rdf_string);
  $item = new Fedora_Item($pid);
  $value = $item->modify_datastream_by_value($rdf_string, 'RELS-EXT', 'RELS-EXT', 'text/xml', $force = FALSE, $logMessage = 'Modified by Drush using Islandora API', $quiet = FALSE);
  if (isset($value)) {
    drush_print('successfully updated RELS-EXT for ' . $pid);
  }
  else {
    drush_print('Error updating RELS-EXT for ' . $pid);
  }
  $item->forget();
}

/**
 *
 * Returns the pids of all pages associated with a book object
 * @param  string $book_pid
 * @return string
 */
function islandora_update_book_rels_get_page_pids($book_pid) {
//return get_sorted_pages($book_pid); //not sure why we have two different functions
  $query = <<<XML
select \$object \$title from <#ri>
	where (\$object <dc:title> \$title
		and \$object <fedora-rels-ext:isMemberOf> <info:fedora/$book_pid>
		and \$object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
	order by \$title
XML;


  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $collection = new CollectionClass($book_pid);
  $contents = $collection->getRelatedItems($book_pid, $query);
  $results_xml = simplexml_load_string($contents);
  $resultsarray = array();
  $book_pids = array();
  foreach ($results_xml->results->result as $result) {
    $book_pids[] = str_replace('info:fedora/', '', $result->object->attributes()->uri);
  }
  return $book_pids;
}

/**
 * @todo pull this function up into an api and remove from here
 * executes a supplied itql query
 * @param string $query

 * @return type 
 */
function islandora_run_query($query) {
  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $cc = new CollectionClass();
  $results = $cc->getRelatedItems(NULL, $query, 1000000);
  if (isset($results)) {//still doesn't tell us if these are valid results
    return $results;
  }
  else {
    drush_print('Error get related items, relationships cannot be updated');
  }
}

?>
