<?php

/**
 * @file
 * This file contains functions used throughout this module.
 *
 *  Some things you may find:
 *    Fetch pages from a book.
 *    Generate Derivatives.
 *    Check for executables.
 *    etc.
 */

/**
 * Checks if it is possible to combined PDF files with GhostScript.
 *
 * @return bool
 *   TRUE if it is possible, FALSE otherwise.
 */
function islandora_book_can_combine_pdf() {
  $gs = variable_get('islandora_book_gs', '/usr/bin/gs');
  return is_executable($gs);
}

/**
 * Appends a number of PDF files onto the given PDF file.
 *
 * @param array $file
 *   The PDF files to append onto.
 * @param array $files
 *   The PDF files to append.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_pdf_append($file, array $files) {
  $temp_file = "$file.temp.pdf";
  copy($file, $temp_file);
  array_unshift($files, $temp_file);
  $ret = islandora_book_combined_pdf($files, $file);
  file_unmanaged_delete($temp_file);
  return $ret;
}

/**
 * Combines the given PDF files into one output file.
 *
 * @param array $files
 *   The PDF files to be combined in order.
 * @param string $out
 *   The absolute path to the combined PDF file.
 *
 * @return bool
 *   TRUE on success, FALSE otherwise.
 */
function islandora_book_pdf_combine(array $files, $out) {
  $gs = variable_get('islandora_book_gs', '/usr/bin/gs');
  $files = implode(' ', $files);
  $command = "{$gs} -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile={$out} {$files}";
  $output = array(); $ret = 0;
  exec($command, $output, $ret);
  if ($ret != 0) {
    $message = 'gs failed to combined PDF<br/>Error: @ret<br/>Command: @command <br/>Output !output';
    $variables = array(
      '@ret' => $ret,
      '@command' => $command,
      '!output' => implode('<br/>', $output),
    );
    watchdog('islandora_book', $message, $variables, WATCHDOG_ERROR);
    return FALSE;
  }
  return TRUE;
}
