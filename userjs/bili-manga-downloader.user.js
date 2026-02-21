// ==UserScript==
// @name         哔哩哔哩漫画下载
// @namespace    http://tampermonkey.net/
// @version      0.0.2
// @description  通过 `HTMLCanvasElement.prototype.toBlob()` 和自动翻页下载 `canvas` 内漫画图片
// @author       JoyWonderful
// @license      GPL-3.0-or-later
// @supportURL   https://github.com/JoyWonderful/JoyWonderful/issues
// @match        https://manga.bilibili.com/mc*
// @icon         https://www.bilibili.com/favicon.ico
// @grant        GM_registerMenuCommand
// @grant        unsafeWindow
// @run-at       document-start
// ==/UserScript==

/////////////////////////
/// anti anti-download //
/////////////////////////

unsafeWindow.console.clear = function(){};// 阻止清除控制台

console.log("begin anti copy", Date());

// 傻逼 bilibili，自动清空控制台；重定义 console.log 等一系列控制台输出；重定义 HTMLCanvasElement, URL 的一系列原型函数
const anti = {
    consoleLog: console.log,
    canvasProtoToBlob: HTMLCanvasElement.prototype.toBlob,
    canvasProtoToDataURL: HTMLCanvasElement.prototype.toDataURL,
    URLCreateObjectURL: URL.createObjectURL,
    URLRevokeObjectURL: URL.revokeObjectURL
}
unsafeWindow.antid = anti;

console.log("end anti copy", Date());

/////////////////////////
/////////////////////////


(function() {
    function sleep(time) {
        return new Promise((resolve) => setTimeout(resolve, time));
    }

    function manga_load_event() {
        var inter_id = unsafeWindow.setInterval(() => { // 移除反人类的鬼东西，这东西也不知加载的事件是啥，就只能重复执行了
            // try {document.querySelector(".manga-reader-ui").remove();} catch {}
            try {document.querySelector(".app-promo").remove();}// “下载 APP，领取更多赚响富力”
            catch {
                if(document.querySelectorAll(".image-item.image-loaded > .image-container > canvas").length > 0) {
                    unsafeWindow.dispatchEvent(new Event("manga:loaded")); // 已经加载完成
                    clearInterval(unsafeWindow.inter_id);
                }
            }
        }, 2500);
        unsafeWindow.inter_id = inter_id;
    }
    // 添加红色分页线（方便自己分页截图）; 去掉菜单阴影; 去掉顶部没用的菜单
    var s = document.createElement("style");
    s.id = "mystylspi";
    s.innerHTML =
    `
    .image-item.image-loaded {border-bottom:4px solid #f00;}
    .mask {filter:none!important;}
    .manga-reader-ui > .navbar-container {display:none;visibility:hidden;}
    `;
    unsafeWindow.addEventListener("DOMContentLoaded", function() {
        document.body.append(s);
        manga_load_event();
    });

    async function download_manga() {
        try {
            var chapter_str = document.querySelector(".info-text :last-child").innerText.substr(2,3); // `第 xxx 话` 的截取
            var pg_cnt = Number(document.querySelector(".info-text :first-child").innerText.replace("P","")); // `xxP` 的数字
            var pages = document.querySelectorAll(".image-item.image-loaded > .image-container > canvas"); // 已加载好图片的 canvas
            console.log(`chapter:${chapter_str}\npage count:${pg_cnt}`);
        }
        catch {
            alert("请等待页面加载完成");
            return false;
        }
        if(pages.length < pg_cnt) {
            alert("剩余 " + String(pg_cnt-pages.length) + " 页未加载，请下滑加载");
            return false;
        }

        var i = 0;
        for(const canv of pages) {
            ++i;
            const fname = `${chapter_str}-${i.toString().padStart(2,"0")}.png`;
            const blob = await new Promise((resolve) => {canv.toBlob(resolve)});
            console.log(blob);
            const bloburl = URL.createObjectURL(blob);
            var down_el = document.createElement("a");
            down_el.download = fname;
            down_el.href = bloburl;
            document.body.appendChild(down_el);
            down_el.click();
            down_el.remove();
            console.log(down_el.download + "\n" + bloburl);
            await sleep(30); // 等一等，不要让储存设备出什么问题...
            URL.revokeObjectURL(bloburl);
        }

        console.log("Download chapter complete.");
        return true;
    }
    var is_auto = false; // 是否开启自动（同时阻塞重复下载）
    async function auto_down() {
        try {
            var pages = document.querySelectorAll(".image-item.image-loaded > .image-container > canvas"); // 已加载的
            var pg_range_input = document.querySelector("input.range-input[type=\"range\"]"); // 那个页数进度条
            var pg_cnt = Number(pg_range_input.max)+1; // e.g. 若有 10 页，则 max="9" min="0"
        }
        catch {
            alert("请等待页面加载完成");
            return false;
        }
        is_auto = false; // 阻塞用
        // 尝试自动翻页
        var i = 0, tryout = 0;
        while(pages.length < pg_cnt) {
            if(i >= pg_cnt) {
                i = 0;
                ++tryout;
                if(tryout >= 10) {
                    alert("自动翻页尝试次数过多，请手动翻页后下载");
                    return false;
                }
            }
            pg_range_input.value = String(i++);
            pg_range_input.dispatchEvent(new Event("change")); // 更改当前页并生效
            await sleep(1000); // 等它加载
            pages = document.querySelectorAll(".image-item.image-loaded > .image-container > canvas");
            console.log(`loadedPageLen:${pages.length}, i:${i}`);
        }

        const res = download_manga();
        if(!res) return false;
        await sleep(100);

        document.querySelector("button.load-next-btn").click(); // 下一页
        // await sleep(5000);
        manga_load_event();
        is_auto = true;
        return true;
    }
    unsafeWindow.addEventListener("manga:loaded", ()=>{
        // 配合 anti anti-download
        // 傻逼 bilibili
        const anti = unsafeWindow.antid;
        unsafeWindow.console.log = anti.consoleLog;
        unsafeWindow.HTMLCanvasElement.prototype.toDataURL = anti.canvasProtoToDataURL;
        unsafeWindow.HTMLCanvasElement.prototype.toBlob = anti.canvasProtoToBlob;
        unsafeWindow.URL.createObjectURL = anti.URLCreateObjectURL;
        unsafeWindow.URL.revokeObjectURL = anti.URLRevokeObjectURL;
        // 继续自动下载
        if(is_auto) auto_down();
    });

    var menuid1 = GM_registerMenuCommand("下载这一话", download_manga, "d");
    var menuid2 = GM_registerMenuCommand("自动下载整本", auto_down, "a");
})();
