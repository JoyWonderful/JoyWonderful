(function() {
    setInterval((function() {
        var b = new URL(window.location);
        if(b.pathname.slice(0, 8) == "/discuss") {
            var a = document.querySelectorAll(".time");
            a.forEach((e) => {
                var c = e.querySelector("time[datetime]")
                if(c) {
                    const d = new Date(c.getAttribute("datetime"))
                    c.innerHTML = new Intl.DateTimeFormat("zh-CN", {year:"numeric", month:"2-digit", day:"2-digit", hour:"2-digit", minute:"2-digit", second:"2-digit"}).format(d);
                }
                else if(e.hasAttribute("title")) {
                    e.innerHTML = e.getAttribute("title");
                }
            });
        }
    }), 1000);
})();
