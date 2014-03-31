<?php

/**
 * @file
 * Derivative code for books.
 */

/**
 * Removes the PDF derivative creation flag.
 * 
 * @param AbstractObject $object
 *   The object to remove the relationship from.
 */
function islandora_book_remove_pdf_flag(AbstractObject $object) {
  $object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'create_pdf', 'true', TRUE);
}

/**
 * Sets the PDF derivative creation flag.
 * 
 * @param AbstractObject $object
 *   The object to set the relationship on.
 */
function islandora_book_set_pdf_flag(AbstractObject $object) {
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'create_pdf', 'true', TRUE);
}
