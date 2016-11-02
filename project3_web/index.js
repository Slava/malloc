'use strict';
m.route.mode = "hash";
var main = document.querySelector('#main');
m.route(main, "/", {
    "/": Main,
});
