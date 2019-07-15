/*eslint-disable no-console */
"use strict";


/********************
* Vars
********************/
// Elements
const
    contentPanel = "content-panel",
    githubLogo = "github-logo",
    headerLogo = "header-logo",
    headerTitle = "header-title",
    menuToggle = "menu-toggle",
    nexusmodsLogo = "nexusmods-logo",
    sideMenu = "side-menu",
    sideMenuItem = "side-menu_item",
    sidePanel = "side-panel"
;
let
    contentBoxID = "",
    contentPanelID = "",
    githubID = "",
    head = document.head,
    headerLogoID = "",
    headerTitleID = "",
    menuToggleID = "",
    nexusmodsLogoID = "",
    sideMenuID = "",
    sideMenuItemClass,
    sidePanelID = ""
;
// Link URLs
const
    externalGithub = "https://github.com/KerberX/Kortex-Mod-Manager",
    externalNexus = "https://www.nexusmods.com/skyrim/mods/90868"
;
let
    sameResource = ""
;
// Options
const
    markedOpt = {
    pedantic: false,
    gfm: true,
    breaks: true,
    smartLists: true,
    smartypants: false,
    xhtml: false
    }
;
// Request Assignments
let
    xhttpCSSFile = xhttpAssign(),
    xhttpMDFile = xhttpAssign(),
    xhttpMenuFile = xhttpAssign()
;
// Strings
const
    // Directories
    dirCSS = "css/",
    dirSrcs = "srcs/",
    dirUp = ".../",
    // File Extensions
    extCSS = ".css",
//  extHTML = ".html",
    extMD = ".md",
    // Filetypes
    css = "css",
    stylesheet = "stylesheet",
    txtcss = "text/css",
    // Functions
    click = "click",
    get = "GET",
    loading = "loading",
    // General    
    auto = "auto",
    badTimes = "_404",
    content = "content",
    external = "external",
    home = "home",
    internal = "internal",
    link = "link",
    menu = "_menu",
    transform = "transform",
    width = "width"
;
// Values
let
    contentHide = "0",
    contentShow = "100",
    menuToggleLeft = "rotate(-180deg)",
    menuToggleRight = "rotate(0deg)",
    sideMenuHide = "0",
    sideMenuShow = "20rem"
;


/********************
* Asset Inits
********************/
function xhttpAssign() {
    if (!window.XMLHttpRequest) return new ActiveXObject("Microsoft.XMLHTTP");
    return new XMLHttpRequest();
}
function initVarAssign(initType) {
    if (initType === home) {
        contentBoxID = document.getElementById(content);
        contentPanelID = document.getElementById(contentPanel);
        githubID = document.getElementById(githubLogo);
        headerLogoID = document.getElementById(headerLogo);
        headerTitleID = document.getElementById(headerTitle);
        menuToggleID = document.getElementById(menuToggle);
        nexusmodsLogoID = document.getElementById(nexusmodsLogo);
        sideMenuID = document.getElementById(sideMenu);
        sidePanelID = document.getElementById(sidePanel);
    }
    if (initType === menu) sideMenuItemClass = document.getElementsByClassName(sideMenuItem);
}
function initListenAssign(initType) {
    if (initType === home) {
        // Functions
        menuToggleID.addEventListener(click, menuClose);
        menuToggleID.addEventListener('mouseenter', elementWillChange);
        menuToggleID.addEventListener('mouseleave', elementWillChangeFinish);
        // Links: External
        githubID.addEventListener(click, function() { linkHandler(externalGithub, external); });
        nexusmodsLogoID.addEventListener(click, function() { linkHandler(externalNexus, external); });
        // Links: Internal
        headerLogoID.addEventListener(click, function() { linkHandler(home, internal); });
        headerTitleID.addEventListener(click, function() { linkHandler(home, internal); });
        // Scrollbar
        OverlayScrollbars(contentPanelID, { className : "os-theme-light" });
    }
    if (initType === menu)
        Object.keys(sideMenuItemClass).forEach(function(i) {
            let datasetPath = sideMenuItemClass[i].dataset.path;
            sideMenuItemClass[i].addEventListener(click, function() { getFilePrimer(datasetPath, content); });
        });
}


/********************
* File Ops
********************/
function linkHandler(location, href) {
    if (href === external) window.open(location, "_blank");
    if (href === internal) getFilePrimer(location, content);
}
function getFilePrimer(resource, type) {
    if (resource !== sameResource) { 
        contentVisibility(contentHide);
        setTimeout(function() {
            importCSSClean();
            getFile(xhttpCSSFile, resource, css);
            getFile(xhttpMDFile, resource, type);
        }, 125);
    }
}
function getFile(file, resource, type) {
    if (type === content) file.open(get, dirUp + dirSrcs + resource + extMD, true); // content block
    if (type === css) file.open(get, dirUp + dirCSS + resource + extCSS, true); // CSS sheet
    if (type === menu) file.open(get, dirUp + dirSrcs + resource + extMD, true); // menu block
    file.onreadystatechange = function() { xhttpReady(this, resource, type); };
    file.send();
    sameResource = resource;
}
function xhttpReady(file, resource, type) {
    if (xhttpStatus(file) === 204) {
        if (type === content) {
            contentBoxID.innerHTML = marked(file.response, markedOpt);
            setTimeout(function() {
                contentVisibility(contentShow);
            }, 175);
        }
        if (type === css) importCSS(resource);
        if (type === menu) {
            getFile(xhttpMDFile, home, content);
            sideMenuID.innerHTML = marked(file.response, markedOpt);
            initVarAssign(menu);
            initListenAssign(menu);
        }
    }
    if (xhttpStatus(file) === 408 && type === content) getFilePrimer(badTimes, content);
}
function xhttpStatus(request) {
    return request.readyState + request.status;
}


/********************
* CSS Handlers
********************/
function importCSSClean() {
    if (head.lastElementChild.type === txtcss) head.removeChild(head.lastElementChild);
}
function importCSS(fileName) {
    let fileNameFull = dirCSS + fileName + extCSS;
    if (fileName !== home) {
        let element = document.createElement(link);  
        element.type = txtcss;
        element.rel = stylesheet;
        element.href = fileNameFull;  
        head.appendChild(element);
    }
}
function contentVisibility(toggle) {
    if (toggle === contentHide) contentPanelID.style.opacity = contentHide;
    if (toggle === contentShow) {
        contentPanelID.style.opacity = contentShow;
        sidePanelID.style.opacity = contentShow;
    }
}


/********************
* Menu Control
********************/
function elementWillChange() {
    menuToggleID.style.willChange = transform;
    sideMenuID.style.willChange = width;
}
function elementWillChangeFinish() {
    setTimeout(function() {
        menuToggleID.style.willChange = auto;
        sideMenuID.style.willChange = auto;
    }, 1000);
}
function menuClose() {
    menuToggleID.removeEventListener(click, menuClose);
    menuToggleID.addEventListener(click, menuOpen);
    setTimeout(function() {
        menuToggleID.style.transform = menuToggleLeft;
    }, 450);
    sideMenuID.style.width = sideMenuHide;
}
function menuOpen() {
    menuToggleID.removeEventListener(click, menuOpen);
    menuToggleID.addEventListener(click, menuClose);
    setTimeout(function() {
        menuToggleID.style.transform = menuToggleRight;
    }, 450);
    sideMenuID.style.width = sideMenuShow;
}


/********************
* Initializations
********************/
function readyUp(caller) {
    if (document.readyState !== loading) return caller();
    if (document.addEventListener) return document.addEventListener("DOMContentLoaded", caller);
    /********************
    * The check should never return null, ever.
    * The script is set to stream load, and the calls above
    * check for states where elements should be searchable.
    ********************/
    return null;
}
readyUp(function() {
    // Setting vars
    initVarAssign(home);
    // Attach listeners
    initListenAssign(home);
    // Init basic content
    getFile(xhttpMenuFile, menu, menu);
});
