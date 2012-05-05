/*
 * jQuery File Upload Plugin JS Example 6.5.1
 * https://github.com/blueimp/jQuery-File-Upload
 *
 * Copyright 2010, Sebastian Tschan
 * https://blueimp.net
 *
 * Licensed under the MIT license:
 * http://www.opensource.org/licenses/MIT
 */

/*jslint nomen: true, unparam: true, regexp: true */
/*global $, window, document */

$(function () {
  'use strict';

  // Initialize the jQuery File Upload widget:
  $('#fileupload').fileupload();

  // Enable iframe cross-domain access via redirect option:
  $('#fileupload').fileupload(
    'option',
    'redirect',
    window.location.href.replace(
      /\/[^\/]*$/,
      '/cors/result.html?%s'
      )
    );


 
    // Load existing files:
    $('#fileupload').each(function () {
      var that = this;
      $.getJSON(book_params['book_callback'], function (result) {
        if (result && result.length) {
          $(that).fileupload('option', 'done')
          .call(that, null, {
            result: result
          });
        }
      });
    });
  


  $('#add_page').click(function(){
    window.top.location.href = book_params['dir_callback'];
  });

  $('#delete_page_files').click(function(){
    $.ajax({
      url: book_params['delete_callback'],
      async:false,
      success: function(data, status, xhr) {
         location.reload();
      },
      error: function() {
        alert("AJAX call failed");
      },
      dataType: 'json'

    });
  
  });

  $('#book_title').text("Add pages to " +book_params['book_title']);

});
