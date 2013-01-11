<?php

/**
 * @file
 *
 * Provides several batch operations for manipulating books.
 *
 * Each batch operation deals with a single page at a time.
 *
 * Batches provided:
 *   Create Book PDF
 *   Perform Page OCR
 *   Create Page Images
 *   Reorder Pages
 *   Delete Pages
 */

/**
 * Sets up a batch operation.
 *
 * @param array $pages
 *   All the pages to operate on.
 * @param array $context
 *   The context of this batch operation.
 * @param array $message
 *   Values are used as parameters to t() and drupal_set_message() that will be
 *   displayed when the batch is finished.
 *
 *   Placeholders include, but are not limited to:
 *     @successful, @failed, @total.
 *
 *  @code
 *    array(
 *      ...
 *      array('Processed @successful of @total. Created @pages pages.', array('@pages' => $pages), 'status')
 *      ...
 *    )
 *  @endcode
 *
 * @return mixed
 *   The current operand the operation will process.
 */
function islandora_book_start_batch_operation(array $operands, array &$context, $message = array()) {
  if (!isset($context['sandbox']['progress'])) {
    if (empty($context['results'])) {
      // "files" will be deleted upon failure or success of the batch.
      // "successful" an array of boolean values indicating operation success.
      // "messages" array of messages to display on finishing the batch
      //    values are used as parameters to t() and drupal_set_message().
      $context['results'] = array_fill_keys(array('files', 'successful', 'messages'), array());
    }
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($operands);
    $context['results']['messages'][] = $message;
  }
  $operand_index = $context['sandbox']['progress'];
  return $operands[$operand_index];
}

/**
 * Ends the batch operation.
 *
 * @param array $context
 *   The context of this batch operation.
 * @param int $count
 *   The number of steps taken in the operation, typically 1.
 * @param array $message
 *   Values are used as parameters to t() and drupal_set_message() that will be
 *   displayed when the batch is finished.
 * @see islandora_book_start_batch_operation
 *
 * @return boolean
 *   TRUE if the batch operation is complete, FALSE otherwise.
 */
function islandora_book_end_batch_operation(array &$context, $count = 1, $message = array()) {
  $context['sandbox']['progress'] += $count;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
    return FALSE;
  }
  $context['results']['messages'][] = $message;
  return TRUE;
}

/**
 * The batch 'finished' callback.
 */
function islandora_book_batch_finished($success, $results, $operations) {
  if ($success) {
    // Delete any lingering files.
    array_walk($results['files'], function($o) {
        if (file_exists($o)) {
          file_unmanaged_delete($o);
        }
      });
    // Build generic place holders for the message.
    $total = count($results['successful']);
    $successful = count(array_filter($results['successful']));
    $failed = $total - $successful;
    $place_holders = array(
      '@successful' => $successful,
      '@failed' => $failed,
      '@total' => $total
    );
    // Display all messages/errors/warnings.
    $messages = array_filter($results['messages']);
    array_walk($messages, function($o) use ($place_holders) {
        $msg = $o[0];
        $vars = isset($o[1]) ? array_merge($o[1], $place_holders) : $place_holders;
        $status = isset($o[2]) ? $o[2] : 'status';
        drupal_set_message(t($msg, $vars), $status);
      });
  }
  else {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
                 '%error_operation' => $error_operation[0],
                 '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * Gets the batch definition for creating a PDF from the given pages from the
 * given book. This also updates each page object with a corresponding pdf.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to include in the PDF.
 * @param array $options
 *   Options to pass into convert.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_create_pdf_batch(FedoraObject $book, array $pages, array $options) {
  return array(
    'operations' => array(
      array('islandora_book_create_pdf_page_batch_operation', array($pages, $options)),
      array('islandora_book_create_pdf_book_batch_operation', array($book)),
    ),
    'finished' => 'islandora_book_batch_finished',
    'title' => t('Creating PDF from @label', array('@label' => $book->label)),
    'init_message' => t('Preparing to create PDF from @label.', array('@label' => $book->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/book.batch.inc',
  );
}

/**
 * Batch operation for converting a page into a PDF.
 *
 * @param array $pages
 *   All the pages to convert.
 * @param array $options
 *   Options to pass into convert.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_create_pdf_page_batch_operation(array $pages, array $options, array &$context) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $page = islandora_book_start_batch_operation($pages, $context);
  $page = islandora_object_load($page);
  $pdf_file = islandora_book_page_derive_pdf($page, $options);
  $context['results']['files'][] = $pdf_file;
  $context['results']['pages'][] = $pdf_file;
  $context['results']['successful'][] = islandora_book_update_datastream($page, $pdf_file, 'PDF');
  islandora_book_end_batch_operation($context);
  $context['message'] = t('Converting page @page of @total into a PDF', array(
                          '@page' => $context['sandbox']['progress'],
                          '@total' => $context['sandbox']['total']));
}

/**
 * Batch operation for combining PDF files.
 *
 * @param FedoraObject $book
 *   The book to store the PDF file.
 * @param string $options
 *   Options to pass into convert.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_create_pdf_book_batch_operation(FedoraObject $book, array &$context) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  islandora_book_start_batch_operation($context['results']['pages'], $context);
  $pdf_file = drupal_realpath("temporary://{$book->id}_PDF.pdf");
  // Combined # of pages at a time.
  $files = array_slice($context['results']['pages'], $progress, 10);
  $append = $context['sandbox']['progress'] != 0;
  $context['results']['successful'][] = $append ? islandora_book_pdf_append($pdf_file, $files) : islandora_book_pdf_combine($files, $pdf_file);
  $finished_message = array('Created PDF with @pages pages.', array('@pages' => count($context['results']['pages'])));
  if (islandora_book_end_batch_operation($context, count($files), $finished_message)) {
    islandora_book_update_datastream($book, $pdf_file, 'PDF');
    $context['results']['files'][] = $pdf_file;
  }
  $context['message'] = t('Adding page @page of @total to the PDF', array(
                          '@page' => $context['sandbox']['progress'],
                          '@total' => $context['sandbox']['total']));
}

/**
 * Gets the batch definition for creating OCR, RAW_HOCR, and ENCODED_OCR
 * datastreams in each of the given page objects.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to process.
 * @param array $options
 *   Options to pass into tesseract/gimp.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_create_ocr_batch(FedoraObject $book, array $pages, array $options) {
  return array(
    'operations' => array(
      array('islandora_book_create_ocr_batch_operation', array($pages, $options)),
    ),
    'finished' => 'islandora_book_batch_finished',
    'title' => t('Performing OCR on @label', array('@label' => $book->label)),
    'init_message' => t('Preparing to perform OCR on @label.', array('@label' => $book->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/book.batch.inc',
  );
}

/**
 * Batch operation for createing the OCR, RAW_HOCR, and ENCODED_OCR datastreams for the given pages.
 *
 * @param array $pages
 *   All the pages to perform OCR on.
 * @param array $options
 *   Options to pass into tesseract/gimp.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_create_ocr_batch_operation(array $pages, array $options, array &$context) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $page = islandora_book_start_batch_operation($pages, $context);
  $page = islandora_object_load($page);
  $context['results']['successful'][] = islandora_book_page_derive_ocr_datastreams($page, $options);
  islandora_book_end_batch_operation($context, 1, array('Performed OCR on @successful of @total pages.'));
  $context['message'] = t('Performing OCR on page @page of @total.', array(
                          '@page' => $context['sandbox']['progress'],
                          '@total' => $context['sandbox']['total']));
}

/**
 * Gets the batch definition for creating TN, JPG, JP2
 * datastreams in each of the given page objects.
 *
 * @param FedoraObject $book
 *   The object the pages belong to.
 * @param array $pages
 *   All the pages to process.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_create_images_batch(FedoraObject $book, array $pages) {
  return array(
    'operations' => array(
      array('islandora_book_create_images_batch_operation', array($book, $pages)),
    ),
    'finished' => 'islandora_book_batch_finished',
    'title' => t('Creating Images for @label', array('@label' => $book->label)),
    'init_message' => t('Preparing to create images for @label.', array('@label' => $book->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/book.batch.inc',
  );
}

/**
 * Batch operation for creating the TN, JPG, JP2 datastreams for the given pages.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to create images for.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_create_images_batch_operation(FedoraObject $book, array $pages, array &$context) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $page = islandora_book_start_batch_operation($pages, $context);
  $page = islandora_object_load($page);
  $context['results']['successful'][] = islandora_book_page_derive_image_datastreams($page);
  if (islandora_book_end_batch_operation($context, 1, array('Created images for @successful of @total pages.'))) {
    islandora_book_update_book_thumbnail($book);
  }
  $context['message'] = t('Creating images for page @page of @total.', array(
                          '@page' => $context['sandbox']['progress'],
                          '@total' => $context['sandbox']['total']));
}

/**
 * Gets the batch definition for re-ordering the given pages from the given book.
 *
 * This will order the pages from 1-#pages, the pages will be assigned a
 * sequence number in the order in which they appear.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to sequence.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_sequence_pages_batch(FedoraObject $book, array $pages) {
  return array(
    'operations' => array(
      array('islandora_book_sequence_pages_batch_operation', array($book, $pages))
    ),
    'finished' => 'islandora_book_batch_finished',
    'title' => t('Sequencing @label\'s pages', array('@label' => $book->label)),
    'init_message' => t('Preparing to sequence @label\'s pages.', array('@label' => $book->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/book.batch.inc',
  );
}

/**
 * Changes the ordering of the pages.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages in the order they should be in.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_sequence_pages_batch_operation(FedoraObject $book, array $pages, array &$context) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $page = islandora_book_start_batch_operation($pages, $context);
  $page = islandora_object_load($page);
  // Pages are offset from 1 rather than 0.
  islandora_book_set_relationship($page->relationships, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', $context['sandbox']['progress'] + 1, TRUE);
  // Assume it worked.
  $context['results']['successful'][] = TRUE;
  if (islandora_book_end_batch_operation($context), array('Sequenced @label', array('@label' => $book->label))) {
    islandora_book_update_book_thumbnail($book);
  }
  $context['message'] = t('Inserting page "@label" (@pid) at position "@pos"', array(
                          '@label' => $page->label,
                          '@pid' => $page->id,
                          '@pos' => $context['sandbox']['progress']));
}

/**
 * Gets the batch definition for deleting the given pages from the given book.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to delete.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_delete_pages_batch(FedoraObject $book, array $pages) {
  return array(
    'operations' => array(
      array('islandora_book_delete_pages_batch_operation', array($book, $pages))
    ),
    'finished' => 'islandora_book_batch_finished',
    'title' => t('Deleting page(s) from @label', array('@label' => $book->label)),
    'init_message' => t('Preparing to delete page(s) from @label.', array('@label' => $book->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/book.batch.inc',
  );
}

/**
 * Batch operation for deleting a page object.
 *
 * @param FedoraObject $book
 *   The book the pages belong to.
 * @param array $pages
 *   All the pages to delete.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_delete_pages_batch_operation(FedoraObject $book, array $pages, array &$context) {
  $page = islandora_book_start_batch_operation($pages, $context);
  $page = islandora_object_load($page);
  $context['message'] = t('Deleting page "@label" (@pid)', array('@label' => $page->label, '@pid' => $page->id));
  $context['results']['successful'][] = islandora_delete_object($page);
  if (islandora_book_end_batch_operation($context, 1, array('Deleted @successful of @total pages.'))) {
    islandora_book_update_book_thumbnail($book);
  }
}
