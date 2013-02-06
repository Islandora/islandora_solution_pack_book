<?php

/**
 * Gets the batch definition for creating a PDF from the given pages.
 *
 * This also updates each page object with a corresponding pdf.
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
      array('islandora_book_create_pdf_page_batch_operation', array(
          $pages, $options)),
      array('islandora_book_create_pdf_book_batch_operation', array(
          $book)),
    ),
    'finished' => 'islandora_paged_content_batch_finished',
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
function islandora_book_create_pdf_page_batch_operation(array $pages, array $options, &$context) {
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $page = islandora_paged_content_start_batch_operation($pages, $context);
  if ($page) {
    $page = islandora_object_load($page);
    $pdf_file = islandora_paged_content_page_derive_pdf($page, $options);
    $context['results']['files'][] = $pdf_file;
    $context['results']['pages'][] = $pdf_file;
    $context['results']['successful'][] = islandora_paged_content_update_datastream($page, $pdf_file, 'PDF');
    islandora_paged_content_end_batch_operation($context);
    $context['message'] = t('Converting page @page of @total into a PDF', array(
                            '@page' => $context['sandbox']['progress'],
                            '@total' => $context['sandbox']['total']));
  }
}

/**
 * Batch operation for combining PDF files.
 *
 * @param FedoraObject $book
 *   The book to store the PDF file.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_create_pdf_book_batch_operation(FedoraObject $book, &$context) {
  module_load_include('inc', 'islandora_paged_content', 'includes/batch');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  if (empty($context['results']['pages'])) {
    return;
  }
  islandora_paged_content_start_batch_operation($context['results']['pages'], $context);
  $pdf_file = drupal_realpath("temporary://{$book->id}_PDF.pdf");
  // Combined # of pages at a time.
  $files = array_slice($context['results']['pages'], $context['sandbox']['progress'], 10);
  $append = $context['sandbox']['progress'] != 0;
  $context['results']['successful'][] = $append ? islandora_book_pdf_append($pdf_file, $files) : islandora_book_pdf_combine($files, $pdf_file);
  $finished_message = array('Created PDF with @pages pages.', array('@pages' => count($context['results']['pages'])));
  if (islandora_paged_content_end_batch_operation($context, count($files), $finished_message)) {
    islandora_paged_content_update_datastream($book, $pdf_file, 'PDF');
    $context['results']['files'][] = $pdf_file;
  }
  $context['message'] = t('Adding page @page of @total to the PDF', array(
                          '@page' => $context['sandbox']['progress'],
                          '@total' => $context['sandbox']['total']));
}

