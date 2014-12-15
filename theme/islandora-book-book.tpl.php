<?php
/**
 * @file
 * Template file to style output.
 */
?>
<?php print theme('islandora_solr_search_return_link'); ?>
<?php if (isset($viewer)): ?>
  <div id="book-viewer">
    <?php print $viewer; ?>
  </div>
<?php endif; ?>
<!-- @todo Add table of metadata values -->
