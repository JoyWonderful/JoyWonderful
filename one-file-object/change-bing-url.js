window.addEventListener("DOMContentLoaded", function(){
  var allLink=document.querySelectorAll("a");
  var urlTest=/^http(s?):\/\/(.*\.)?bing.com\/ck/;
  allLink.forEach((link) => {
    if(urlTest.test(link.href)) {
      try {
        var tmp = atob(new URL(link.href).searchParams.get("u").replace(/^a1/, "").replace(/_/g, "/").replace(/-/g, "+"));
        //console.log(tmp);
        link.href = tmp;
      }
      catch(err) {
        console.error("Error change link on" + link);
      }
    }
  });
  //console.log(allLink);
});