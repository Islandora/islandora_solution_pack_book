<?php

/**
 * @file
 *
 * @todo: Document
 */

?>
<div class="islandora islandora-book-pages">
  <?php $row_field = 0; ?>
  <?php foreach($pages as $page): ?>
    <div class="islandora-book-page-object islandora-book-page-list-item clearfix">
      <dl class="<?php print $page['class']; ?>">
        <dt><?php print $page['image']; ?></dt>
        <dd class="islandora-book-page-object <?php print $page['class']; ?> <?php print $row_field == 0 ? 'first' : ''; ?>">
          <strong><?php print $page['link']; ?></strong>
        </dd>
        <?php if (isset($page['DC']['dc:description']['value'])): ?>
          <dd class="<?php print $page['DC']['dc:description']['class']; ?>">
            <?php print $page['DC']['dc:description']['value']; ?>
          </dd>
        <?php endif; ?>
      </dl>
    </div>
    <?php $row_field++; ?>
  <?php endforeach; ?>
</div>
