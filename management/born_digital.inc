<?php

function create_borndigital($book_pid){
  module_load_include('inc', 'islandora_book', 'management/add_pages');
  $content = drupal_get_form('book_add_pages_form', $book_pid, 'islandora:bd_pageCModel');
  return $content;

}