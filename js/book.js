Drupal.behaviors.book = function(context){
  this.imagePreview = function(){
    /* CONFIG */

    xOffset = 10;
    yOffset = 30;

    // these 2 variable determine popup's distance from the cursor
    // you might want to adjust to get the right result

    /* END CONFIG */
    $("a.fedora_image").hover(function(e){
      this.t = this.title;
   
      this.title = "";
      var c = (this.t != "") ? "<br/>" + this.t : "";
   
      $('body').append("<p id='thumbnail'><img src='"+ this.href +"/TN' alt='Image preview' />"+ c +"</p>");
      $("#thumbnail")
      .css("top",(e.pageY - xOffset) + "px")
      .css("left",(e.pageX + yOffset) + "px")
      .fadeIn("fast");
    
    },
    function(){
      this.title = this.t;
      $("#thumbnail").remove();
    });

    $("a.fedora_image").mousemove(function(e){
      $("#thumbnail")
      .css("top",(e.pageY - xOffset) + "px")
      .css("left",(e.pageX + yOffset) + "px");
    });
  };

  imagePreview();
  var label = $('#file_view');
  label.hide();

  $("#file_label").toggle( function(event){
    label.show('fast');
    event.preventDefault();
  },
  function (event) {
    label.hide('fast');
    event.preventDefault();
  });


};

