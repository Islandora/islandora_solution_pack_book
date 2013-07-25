<?php
/**
 * @file
 * Template file to style output.
 */
?>
<?php if (isset($viewer)): ?>
  <div id="book-viewer">
    <?php print $viewer; ?>
  </div>
<?php endif; ?>
<div class="islandora-book clearfix" itemscope itemtype="http://schema.org/Book">
	<fieldset class="collapsible collapsed islandora-book-metadata">
	  <legend><span class="fieldset-legend"><?php print t('Details'); ?></span></legend>
	    <div class="fieldset-wrapper">
	      <dl xmlns:dcterms="http://purl.org/dc/terms/" class="islandora-inline-metadata islandora-book-fields">
	        <?php $row_field = 0; ?>
	        <?php foreach($dc_array as $key => $value): ?>
	          <dt property="<?php print $value['dcterms']; ?>" content="<?php print $value['value']; ?>" class="<?php print $value['class']; ?><?php print $row_field  == 0 ? ' first' : ''; ?>">
	            <?php print $value['label']; ?>
	          </dt>
	          <dd class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
	            <?php print $value['value']; ?>
	          </dd>
	          <?php $row_field++; ?>
	        <?php endforeach; ?>
	      </dl>
	    </div>
	</fieldset>
</div>