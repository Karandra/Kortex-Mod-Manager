/*eslint-disable no-console */
"use strict";


/********************
* Vars
********************/
// Elements
const
    contentPanel = "content-panel",
    header = "header",
    headerLogo = "header-logo",
    headerTitle = "header-title",
    linkLogo = "link-logo",
    menuToggle = "menu-toggle",
    scrollbarBox = "os-viewport",
    sideMenu = "side-menu",
    // sideMenuItem = "side-menu_item",
    sideMenuItems = "side-menu_items",
    sidePanel = "side-panel"
;
let
    contentBoxID = "",
    contentLinkItems,
    contentPanelID = "",
    head = document.head,
    headerID = "",
    headerLogoID = "",
    headerLogoLink = "",
    headerTitleID = "",
    headerTitleLink = "",
    linkLogoClass = "",
    loadingID = "",
    menuToggleID = "",
    scrollbarBoxID = "",
    sideMenuID = "",
    sideMenuItemTag,
    sideMenuItemsClass = "",
    sidePanelID = ""
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
    urlFixed = null,
    urlHash = null,
    urlParam = null,
    urlRaw = null,
    xhttpCSSFile = xhttpAssign(),
    xhttpMDFile = xhttpAssign(),
    xhttpMenuFile = xhttpAssign()
;
// Strings
const
    // Directories
    dirCSS = "css/",
    dirSrcs = "srcs/",
    dirUtil = "util/",
    // File Extensions
    extCSS = ".css",
    extMD = ".md",
    // Filetypes
    css = "css",
    stylesheet = "stylesheet",
    txtcss = "text/css",
    // Functions
    click = "click",
    contentUncloakTime = 200,
    contentCloakTime = 150,
    get = "GET",
    loading = "loading",
    menuChangeTimeout = 1000,
    menuSlideFlip = 450,
    mouseEnter = "mouseenter",
    mouseLeave = "mouseleave",
    reqFail = 408,
    reqPass = 204,
    // General    
    auto = "auto",
    badTimes = "404",
    content = "content",
    entry = "entry",
    home = "home",
    homeParam = "?page=home",
    link = "link",
    menu = "menu",
    menuParam = "?page=menu",
    reentry = "reentry",
    transform = "transform",
    width = "width",
    // Regex
    regexHash = /#[\w\d-]*/,
    regexHREF = /(?:http:)|(?:https:)/,
    regexParam = /(?:\?page=)[\w\d-]*/,
    regexParamPage = /(?:\?page=)/
;
let
    sameLocation = ""
;
// Style Values
const
    contentHide = "0",
    contentShow = "100",
    displayBlock = "block",
    displayNone = "none",
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
    switch(initType) {
        case entry:
            // Get URL pieces
            urlRaw = window.location;
            urlParam = urlRaw.href.match(regexParam);
            urlHash = urlRaw.href.match(regexHash);
            // Assemble URL pieces      
            urlFixed = urlRaw.origin;
            urlFixed += urlRaw.pathname;
            if (urlParam !== null) urlFixed += urlParam;
            if (urlHash !== null) urlFixed += urlHash;
            break;
        case home:
            contentBoxID = document.getElementById(content);
            contentPanelID = document.getElementById(contentPanel);
            headerID = document.getElementById(header);
            headerLogoID = document.getElementById(headerLogo);
            headerLogoLink = headerLogoID.querySelector("A");
            headerTitleID = document.getElementById(headerTitle);
            headerTitleLink = headerTitleID.querySelector("A");
            linkLogoClass = headerID.getElementsByClassName(linkLogo);
            loadingID = document.getElementById(loading);
            menuToggleID = document.getElementById(menuToggle);
            sideMenuID = document.getElementById(sideMenu);
            sideMenuItemsClass = document.getElementsByClassName(sideMenuItems)[0];
            sidePanelID = document.getElementById(sidePanel);
            break;
        case menu:
            sideMenuItemTag = sideMenuItemsClass.getElementsByTagName("A");
            break;
        default: 
            contentLinkItems = contentBoxID.getElementsByTagName("A");
            scrollbarBoxID = contentPanelID.getElementsByClassName(scrollbarBox)[0];
    }
    // Init listeners post-var assign
    if (initType !== entry) initListenAssign(initType);
}
function initListenAssign(initType) {
    // Should only receive content, home, or menu
    let linkType;
    switch (initType){
        case badTimes:
            break;
        case home:
            // Functions
            menuToggleID.addEventListener(click, menuClose);
            menuToggleID.addEventListener(mouseEnter, elementWillChange);
            menuToggleID.addEventListener(mouseLeave, elementWillChangeFinish);
            // Links: External
            Object.keys(linkLogoClass).forEach(function(i) {
                let linkLogoHREF = linkLogoClass[i].querySelector("A");
                let linkHREFParam = linkLogoHREF.getAttribute("href");
                linkLogoClass[i].addEventListener(click, function(event) { event.preventDefault(); linkHandler(linkHREFParam, content); });
            });
            // linkLogoClass.addEventListener(click, function() { event.preventDefault(); linkHandler(externalGithub); });
            // Links: Internal
            headerLogoLink.addEventListener(click, function() { event.preventDefault(); linkHandler(this.search, content); });
            headerTitleLink.addEventListener(click, function() { event.preventDefault(); linkHandler(this.search, content); });
            // Scrollbar
            if (OverlayScrollbars) OverlayScrollbars(contentPanelID, { className : "os-theme-light" });
            break;
        default:
            if (initType === content) linkType = contentLinkItems;
            if (initType === menu) linkType = sideMenuItemTag;
            Object.keys(linkType).forEach(function(i) {
                let linkHREFParam = linkType[i].getAttribute("href");
                linkType[i].addEventListener(click, function(event) { event.preventDefault(); linkHandler(linkHREFParam, content); });
            });
    }
}


/********************
* File Ops
********************/
function linkHandler(inputHREF, accessType) {
// TODO: Correct scroll position to top when navigating from a page that had an anchor
    // Abort
    if (inputHREF === null) return null;

    // External links out
    if (regexHREF.test(inputHREF)) return window.location.assign(inputHREF);

    // Internal links in
    let locationString = inputHREF.toString();
    let locationTrimString = locationString.replace(regexParamPage, "").replace(regexHash, "");

    // Proceed only if we aren't headed to the same page
    if (locationTrimString !== sameLocation) {
        let locationType;
        let xhttpInstance = xhttpMDFile;
        switch (accessType) {
            case menu: locationType = menu; xhttpInstance = xhttpMenuFile; break;
            case badTimes: locationType = badTimes; break;
            default: locationType = content;
        }

        // Browser history manipulation
        if (accessType === content) window.history.pushState(null, null, locationString);
        if (accessType === entry) window.history.replaceState(null, null, locationString);

        // Hash links navigation
        if (regexHash.test(inputHREF)) {
            if (!regexParam.test(inputHREF)) return window.location.assign(inputHREF);
            urlHash = inputHREF.match(regexHash);
        }

        // Engage cloaking device for content swap
        contentVisibility(contentHide);
        return setTimeout(function() {
            getFile(xhttpCSSFile, locationTrimString, css);
            getFile(xhttpInstance, locationTrimString, locationType);
        }, contentCloakTime);
    }
    return null;
}

function getFile(file, resource, type) {
    switch (type) {
        case css:
            file.open(get, dirCSS + resource + extCSS, true);
            break;
        case menu:
        case badTimes:
            file.open(get, dirSrcs + dirUtil + resource + extMD, true);
            break;
        default:
            file.open(get, dirSrcs + resource + extMD, true);
    }
    file.onreadystatechange = function() { xhttpReady(this, resource, type); };
    file.send();
}

function xhttpReady(file, resource, type) {
    let fileCheck = file.readyState + file.status;
    if (fileCheck === reqPass)
        switch (type) {
            case css:
                importCSSClean();
                importCSS(resource);
                break;
            case menu:
                sideMenuItemsClass.innerHTML = marked(file.response, markedOpt);
                initVarAssign(menu);
                break;
            default:
                contentBoxID.innerHTML = marked(file.response, markedOpt);
                initVarAssign(type);
                scrollbarBoxID.scrollTo(0, 0);
                if (urlHash !== null) {
                    location.assign(urlHash);
                    urlHash = null;
                }
                setTimeout(contentVisibility(contentShow), contentUncloakTime);
                sameLocation = resource;
                if (type === badTimes) sameLocation = "";
        }
    if (fileCheck === reqFail)
        switch (type) {
            case css:
                importCSSClean();
                break;
            default:
                linkHandler(badTimes, badTimes);
        }
}


/********************
* CSS Handlers
********************/
function importCSSClean() {
    if (head.lastElementChild.type === txtcss) head.removeChild(head.lastElementChild);
}

function importCSS(fileName) {
    if (fileName !== home) {
        let element = document.createElement(link);  
        element.type = txtcss;
        element.rel = stylesheet;
        element.href = dirCSS + fileName + extCSS;  
        head.appendChild(element);
    }
}

function contentVisibility(toggle) {
    if (toggle === contentHide) {
        contentPanelID.style.opacity = contentHide;
        loadingID.style.display = displayBlock;
        loadingID.style.opacity = contentShow;
    }
    if (toggle === contentShow) {
        contentPanelID.style.opacity = contentShow;
        sidePanelID.style.opacity = contentShow;
        loadingID.style.opacity = contentHide;
        setTimeout(function() {
            loadingID.style.display = displayNone;
        }, contentCloakTime);
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
    }, menuChangeTimeout);
}

function menuClose() {
    menuToggleID.removeEventListener(click, menuClose);
    menuToggleID.addEventListener(click, menuOpen);
    setTimeout(function() {
        menuToggleID.style.transform = menuToggleLeft;
    }, menuSlideFlip);
    sideMenuID.style.width = sideMenuHide;
}

function menuOpen() {
    menuToggleID.removeEventListener(click, menuOpen);
    menuToggleID.addEventListener(click, menuClose);
    setTimeout(function() {
        menuToggleID.style.transform = menuToggleRight;
    }, menuSlideFlip);
    sideMenuID.style.width = sideMenuShow;
}


/********************
* Initializations
********************/
function readyUtil(caller) {
    if (document.readyState !== loading) return caller();
    if (document.addEventListener) return document.addEventListener("DOMContentLoaded", caller);
    /********************
    * This check should never return null, ever.
    * The script is set to dynamically load content, and the calls
    * above check for states where core elements should be searchable.
    ********************/
    return null;
}

readyUtil(function() {
    // Init vars and listeners
    initVarAssign(home);
    // Analyze URL and repair if necessary
    initVarAssign(entry);
    if (urlFixed !== urlRaw.toString()) window.location = urlFixed;
    // Load menu
    linkHandler(menuParam, menu);
    // Load destination
    if (urlParam !== null) return linkHandler(urlParam, reentry);
    return linkHandler(homeParam, entry);
});

window.onpopstate = function() {
    // Analyze URL
    initVarAssign(entry);
    // Build input
    let urlHistory = String();
    if (urlParam !== null) urlHistory += urlParam;
    if (urlHash !== null) urlHistory += urlHash;
    // Load destination
    linkHandler(urlHistory, null);
};
