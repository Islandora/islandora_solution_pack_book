<?php

/**
 * @file
 *
 * @todo: Document
 */

?>
<div class="islandora islandora-book-pages">
  <div class="islandora-book-pages-grid clearfix">
  <?php foreach($pages as $page): ?>
    <dl class="islandora-book-page-object <?php print $page['class']; ?>">
        <dt class="islandora-book-page-thumb"><?php print $page['image']; ?></dt>
        <dd class="islandora-book-page-caption"><?php print $page['link']; ?></dd>
    </dl>
  <?php endforeach; ?>
  </div>
</div>
