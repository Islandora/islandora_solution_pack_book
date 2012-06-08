

$(function () {
  $.urlParam = function(name){
    var results = new RegExp('[\\?&]' + name + '=([^&#]*)').exec(window.location.href);
    if (!results)
    {
      return 0;
    }
    return results[1] || 0;
  }
  var PID = $.urlParam('PID');

  var here = window.location.toString();
  var splitter = here.indexOf('/sites/');
  if(splitter > 0){
    splitter = '/sites/';
  }else{
    splitter = '/modules';
  }
  var base = here.split(splitter);
  base = base[0];
  var url = base + '/islandora/book/upload_setup/' + PID;
  $.ajax({
    url: url,
    async:false,
    success: function(data, status, xhr) {
      book_params = data;
    },
    error: function() {
      alert("AJAX call failed");
    },
    dataType: 'json'
  });

  $('#back_to_book').click(function(e){
    e.preventDefault();

    window.top.location.href = base + '/fedora/repository/' + PID;
  });

});
