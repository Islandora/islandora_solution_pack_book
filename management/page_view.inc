<?php
//not yet implemented
function show_pages($book_pid) {
  global $queryClass;
  islandora_solr_search_init();
  $params = array('sort' => 'rels.isPageNumber');
  $searchString = 'rels.isPageOf:islandora\:1709';
  $params = array('sort' => 'rels.isPageNumber asc');
  $queryResult = $queryClass->buildAndExecuteQuery($searchString, '-', $dismax, $params);

  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');
  $resultsClass = new PageViewSolrResults();
  $output = $resultsClass->displayResults($queryClass);

  return $output;
}

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');
class PageViewSolrResults extends IslandoraSolrResults {
  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;
  function PageViewSolrResults() {
    module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  }
  function printResults($results) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  
    global $base_url;
    $recordStart = (int) $results->response->start;
    if (empty($results)) {
      return "no results";
    }
    $rawResponse = $results->getRawResponse();
    $responseArray = json_decode($rawResponse, true);

    $docs = $responseArray['response']['docs'];
    foreach ($docs as $doc) {
      $rows = array();
      foreach ($doc as $field => $value){
      }
      $output .= theme_table(null, $rows);
    }
    return '<div class ="islandora-solr-search">' . $output . '</div>';
  }

}
