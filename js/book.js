Drupal.behaviors.book = function(context){
  $("a.fedora_image img ").hover(function() {
    $(this).attr("width","120");
  }, function() {
    $(this).attr("width","20");
  });

};

