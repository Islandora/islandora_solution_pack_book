<?php

/**
 * @file
 *
 * Renders all the pages within a given book as a grid of thumbnails.
 */

?>
<div>
  <div class="islandora-book-pages clearfix">
    <span class="islandora-book-pages-display-switch">
     <?php print theme('links', array('links' => $view_links, 'attributes' => array('class' => array('links', 'inline'))));?>
    </span>
    <?php print $pager; ?>
    <?php print $content; ?>
    <?php print $pager; ?>
  </div>
</div>
