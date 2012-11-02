<?php

/**
 * @file
 * Islandora book
 */

/**
 * Islandora Book
 */
class IslandoraBook {

  /**
   * Constructor
   * @param string $pid
   */
  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  /**
   * Show field sets
   * @global string $base_url
   * @global user $user
   * @return string
   */
  public function showFieldSets() {
    global $user;
    global $base_url;
    module_load_include('inc', 'islandora_book', 'book_pack_utils');
    module_load_include('inc', 'islandora_book', 'add_pages');
    module_load_include('module', 'islandora_book');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $path = drupal_get_path('module', 'islandora_book');
    drupal_add_css($path . '/css/islandora_book.css');

    drupal_add_js($path . '/js/book.js?stopcache');
    global $base_url;
    $show_purge_tab = (!empty($_POST['form_id']) && ($_POST['form_id'] == 'islandora_book_purge_pages_form'));
    $page_number = 1;
    $pages_selected = FALSE;
    $qstring = $_GET['q'];
    $qparts = explode('/', $qstring);
    $tail = end($qparts);
    if (is_numeric($tail)) {
      $pages_selected = TRUE;
      $page_number = $tail;
    }
    $class = new CollectionClass();
    $query = <<<XML
select \$object \$title  from <#ri>
where \$object <fedora-rels-ext:isMemberOf> <info:fedora/$this->pid>
and \$object <fedora-model:state> <fedora-model:Active>
and (\$object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> \$title
or \$object <http://islandora.ca/ontology/relsext#isSequenceNumber> \$title)
order by \$title
XML;
    $results = $class->getRelatedItems($this->pid, $query);
    $page_items = $class->renderCollection($results, $this->pid, NULL, NULL, $page_number);
    $page_pids = get_page_pids($this->pid);

    if (count($page_pids) > 0) {
      $hasPage = TRUE;
    }
    $tabset = array();

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }
    module_load_include('inc', 'islandora_bookviewer', 'islandora_bookviewer');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $oh = new ObjectHelper();
    //This is a bit of a hack but allows us to handle two viewers in one solution pack
    //ideally we would do module invoke_all and then implement the viewers in a module
    //but two modules for 4 lines of code seems a bit much.
    $sequence_number = (empty($_GET['startpage'])) ? '1' : $_GET['startpage'];
    $cmodels = $oh->get_content_models_list($this->pid);
    foreach ($cmodels as $cmodel) {
      if ($cmodel->pid == "islandora:iaBookCModel") {
        $bookviewer = new IslandoraBookviewer($this->pid);
        $html = $bookviewer->showAjaxBookViewer('mainpage.php', $sequence_number);
      }
      else {
        $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->pid . '/islandora:viewerSdef/getViewer' . $qs;
        $html = '<iframe src="' . $viewer_url . '" scrolling="0" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';
      }
    }
    $tabset['pages_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Pages'),
      '#selected' => $pages_selected,
      '#content' => $page_items,
      '#weight' => 15,
    );

    $tabset['first_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('About'),
      '#weight' => 1,
      // This will be the content of the tab.
      //'#content' => islandora_book_create_book_view($this->pid),
      '#content' => $this->create_book_view(),
    );

    if ($hasPage) {
      $tabset['second_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Reading Edition'),
        '#content' => $html,
        '#weight' => -10,
      );
    }

    return $tabset;
  }

  function create_book_viewx() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'islandora_book', 'book_pack_utils');

    $page_pids = get_sorted_pages($pid);
    if (count($page_pids) > 0) {
      $has_pages = TRUE;
    }
    $item = new Fedora_Item($this->pid);
    $img = theme_image("$base_url/fedora/repository/" . $this->pid . "/TN", '', NULL, NULL, FALSE);
    $links = array();
    $links[] = l(t('Metadata'), "islandora/page/manage/metadata/$this->pid/");
    $links[] = l(t('Management'), "islandora/book/management/$this->pid/");
    $links = array_merge($links, module_invoke_all('islandora_book_get_bookview_links', $this->pid));

    $headers = array($item->objectProfile->objLabel, "");
    $rows = array(array($img, theme_item_list($links)));
    return theme_table($headers, $rows);
  }

  function create_book_view() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'ObjectDetails');
    $headers = array($item->objectProfile->objLabel, "");
    $item = new Fedora_Item($this->pid);
    $img = theme_image("$base_url/fedora/repository/" . $this->pid . "/TN", '', NULL, NULL, FALSE);
    $output = "";
    if (user_access('manage book object')) {
      $links = array();
      $links[] = l(t('Edit Metadata'), "fedora/repository/editmetadata/$this->pid/metadata");
      $links[] = l(t('Manage Book'), "islandora/book/management/$this->pid/");
      $links = array_merge($links, module_invoke_all('islandora_book_get_bookview_links', $this->pid));
      $output .= theme_item_list($links);
    }

    $output .= fedora_repository_object_details_XSLT($item);

    $headers = array($item->objectProfile->objLabel, "");
    $rows = array(array($img, $output));
    return theme_table($headers, $rows);
  }

}

