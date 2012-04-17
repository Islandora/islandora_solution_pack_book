Drupal.behaviors.book = function(context){
  $('.cwrc_submit').hide();
  $(".jumpmenu").change(function() {
    var val = ($('.jumpmenu :selected').attr('value'));
    if (val != '') {
      //location.href=val;
      window.open(val);
    }
  });
};

