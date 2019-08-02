// ********************\
// * Vars
// ********************/
// Elements
const
	contentPanel = "content-panel",
	header = "header",
	headerLogo = "header-logo",
	headerTitle = "header-title",
	linkLogo = "link-logo",
	menuToggle = "menu-toggle",
	menuToggleIcon = "menu-toggle-icon",
	menuToggleIconFadeIn = "menu-toggle-icon-filter-enter",
	menuToggleIconFadeOut = "menu-toggle-icon-filter-exit",
	scrollbarBox = "os-viewport",
	sideMenu = "side-menu",
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
	menuToggleIconID = "",
	menuToggleIconFadeInID = "",
	menuToggleIconFadeOutID = "",
	scrollbarBoxID = "",
	sideMenuID = "",
	sideMenuItemTag,
	sideMenuItemsClass = "",
	sidePanelID = ""
	;
// Options
const
	cssVarsOpt = {
		silent: false,
		watch: true
	},
	markedOpt = {
		pedantic: false,
		gfm: true,
		breaks: true,
		smartLists: true,
		smartypants: false,
		xhtml: false
	},
	scrollbarOpt = {
		className: "os-theme-block-light",
		scrollbars: {
			clickScrolling: true,
			snapHandle: false
		},
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
// Resource Constructors
const
	anchorLinkStart = "<a ",
	anchorLinkHREF = "href=\"",
	anchorLinkTitle = "\" title=\"",
	anchorSpanStart = "<span ",
	anchorClass = "class=\"",
	anchorSpanClass = "anchor fas fa-link",
	anchorSpanEnd = "</span>",
	anchorLinkEnd = "</a>",
	anchorCloseTag = "\">",
	anchorHash = "#"
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
	menuToggleLeft = "rotate(0deg)",
	menuToggleRight = "rotate(180deg)",
	sideMenuHide = "0",
	sideMenuShow = "20rem"
	;
// Switches
let
	attemptCSS = 0,
	loadedCSS = false,
	loadedMD = false,
	loadedMenu = false,
	maxAttemptCSS = 3
	;


// ********************\
// * Asset Inits
// ********************/
/**
 * @description Used to generate an object loads and holds a file for manipulation.
 *
 * @returns `XMLHttpRequest` - Modern browsers
 * @returns `ActiveXObject("Microsoft.XMLHTTP")` - IE
 */
function xhttpAssign() {
	if (!window.XMLHttpRequest) return new ActiveXObject("Microsoft.XMLHTTP");
	return new XMLHttpRequest();
}
/**
 * @description Assigns vars after page has been rendered. 
 *	Is called on script initialization, as well as after internal page swap. 
 *	Automatically runs listener assignment after vars are filled.
 * 
 * - case `entry` - Called on page init as well as history change. Pieces together current `window.location`.
 * - case `home` - Assigns core page elements. Only run on page init.
 * - case `menu` - Assigns menu elements. Only run on page init.
 * - default - Called every time a new content block is loaded.
 *
 * @param {*} initType - Category to run assignments for.
 */
function initVarAssign(initType) {
	switch (initType) {
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
			menuToggleIconID = document.getElementById(menuToggleIcon);
			menuToggleIconFadeInID = document.getElementById(menuToggleIconFadeIn);
			menuToggleIconFadeOutID = document.getElementById(menuToggleIconFadeOut);
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
			if (initType === content) {
				let h1Anchors = contentBoxID.getElementsByTagName("H1");
				let h2Anchors = contentBoxID.getElementsByTagName("H2");
				insertHeaderAnchors(h1Anchors);
				insertHeaderAnchors(h2Anchors);
			}
	}
	// Init listeners post-var assign
	if (initType !== entry) initListenAssign(initType);
}
/**
 * @description Applies event listeners to appropriate objects.
 * 
 * - case `badTimes` - Breaks listener assignment process on 404.
 * - case `home` - Assigns listeners to core page elements. Only run on page init.
 * - default - Assigns listeners on menu init, as well as every time a new content block is written.
 * 
 * @param {*} initType - Category to run assignments for.
 */
function initListenAssign(initType) {
	// Should only receive content, home, or menu
	let linkType;
	switch (initType) {
		case badTimes:
			break;

		case home:
			// Functions
			menuToggleID.addEventListener(click, menuClose);
			menuToggleID.addEventListener(mouseEnter, elementWillChange);
			menuToggleID.addEventListener(mouseLeave, elementWillChangeFinish);

			// Links: External
			Object.keys(linkLogoClass).forEach(function (i) {
				let linkLogoHREF = linkLogoClass[i].querySelector("A");
				let linkHREFParam = linkLogoHREF.getAttribute("href");
				linkLogoClass[i].addEventListener(click, function (event) {
					event.preventDefault();
					linkHandler(linkHREFParam, external);
				});
			});

			// Links: Internal
			headerLogoLink.addEventListener(click, function () {
				event.preventDefault();
				linkHandler(this.search, content);
			});
			headerTitleLink.addEventListener(click, function () {
				event.preventDefault();
				linkHandler(this.search, content);
			});

			// Scrollbar
			if (OverlayScrollbars) OverlayScrollbars(contentPanelID, scrollbarOpt);
			break;

		default:
			if (initType === content) linkType = contentLinkItems;
			if (initType === menu) linkType = sideMenuItemTag;
			Object.keys(linkType).forEach(function (i) {
				let linkHREFParam = linkType[i].getAttribute("href");
				linkType[i].addEventListener(click, function (event) {
					event.preventDefault();
					linkHandler(linkHREFParam, content);
				});
			});
	}
}
/**
 * @description Builds H1 and H2 hash anchor links after content is written.
 * @todo Real ugly string construction. Replace with `createElement()`.
 *
 * @param {*} elementList - Array of heading elements to process.
 */
function insertHeaderAnchors(elementList) {
	Object.keys(elementList).forEach(function (i) {
		let headerID = elementList[i].getAttribute("id");
		let headerAnchorString =
			anchorLinkStart + anchorLinkHREF + anchorHash + headerID + anchorLinkTitle + headerID + anchorCloseTag
			+ 
			anchorSpanStart + anchorClass + anchorSpanClass + anchorCloseTag + anchorSpanEnd
			+
			anchorLinkEnd;
		elementList[i].insertAdjacentHTML("beforeend", headerAnchorString);
	});
}


// ********************\
// * File Ops
// ********************/
/**
 * @description Intercepts clicked link events to manually sort and process. 
 *	Modifies browser history based on `accessType`.
 *
 * @param {*} inputHREF - Link to process.
 * @param {*} accessType - Category for filtering.
 * 
 * @returns `null` - Aborts on bad link input.
 * @returns `accessType === external` `window.location.assign` - Navigates external links.
 * @returns `regexHREF` `window.location.assign` - Navigates external links.
 * @returns `regexHash` `!regexParam` `window.location.assign` - Scrolls page to pointed hash anchor.
 * @returns `setTimeout()` - Begins content swap processing after hiding block.
 */
function linkHandler(inputHREF, accessType) {
	// Abort
	if (inputHREF === null) return null;

	// External links out
	if (regexHREF.test(inputHREF) || accessType === external) return window.location.assign(inputHREF);

	// Internal links in
	let locationString = inputHREF.toString();
	let locationTrimString = locationString.replace(regexParamPage, "").replace(regexHash, "");

	// Proceed only if we aren't headed to the same page
	if (locationTrimString !== sameLocation) {
		// Check attributes of input for appropriate assignments
		let locationType;
		let xhttpInstance = xhttpMDFile;
		switch (accessType) {
			case menu:
				locationType = menu;
				xhttpInstance = xhttpMenuFile;
				break;
			case badTimes:
				locationType = badTimes;
				break;
			default:
				locationType = content;
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
		// Wait 150ms while content opacity finishes dropping
		return setTimeout(function () {
			getFile(xhttpCSSFile, locationTrimString, css);
			getFile(xhttpInstance, locationTrimString, locationType);
		}, contentCloakTime);
	}
	return null;
}
/**
 * @description Builds file request for load.
 * - case `css` - Builds stylesheet URL.
 * - case `menu` - Builds menu URL.
 * - case `badTimes` - Builds 404 URL.
 * - default - Builds content URL.
 * 
 * @param {*} request - XMLHttpRequest object instance.
 * @param {*} resource - File name to load.
 * @param {*} type - Flag to apply correct path location.
 */
function getFile(request, resource, type) {
	// Construct path to file
	switch (type) {
		case css:
			loadedCSS = false;
			request.open(get, dirCSS + resource + extCSS, true);
			break;

		case badTimes:
			loadedMD = false;
			request.open(get, dirSrcs + dirUtil + resource + extMD, true);
			break;

		case menu:
			request.open(get, dirSrcs + dirUtil + resource + extMD, true);
			break;

		default:
			loadedMD = false;
			request.open(get, dirSrcs + resource + extMD, true);
	}

	// Assign ready state watch and initiate load
	request.onreadystatechange = function () { xhttpReady(this, resource, type); };
	request.send();
}
/**
 * @description Processes file based on ready state and status of file load. 
 * - `requestCheck` - 204 (State 200, Status 4) pass, 408 (State 404, Status 4) fail.
 * - case `css` - Dump any previous page-unique stylesheet and append new one to `<head>`.
 * - case `menu` - Writes menu block and begins menu var assignments.
 * - default - Writes any content block files including 404 page, begins general var assignments, forces 
 *	scroll to top, navigates to hash anchor if found, unhides content block, and fills `sameLocation` var.
 *
 * @param {*} request - XMLHttpRequest object instance.
 * @param {*} resource - File name.
 * @param {*} type - Flag to direct appropriate file actions.
 */
function xhttpReady(request, resource, type) {
	let requestCheck = request.readyState + request.status;

	// Process when file is ready, writing to page
	if (requestCheck === reqPass)
		switch (type) {
			case css:
				// Clean any previous unique CSS out, insert new, flag load success
				importCSSClean();
				importCSS(resource);
				loadedCSS = true;
				break;

			case menu:
				// Write menu to page, set vars and listeners, flag load success
				sideMenuItemsClass.innerHTML = marked(request.response, markedOpt);
				initVarAssign(menu);
				loadedMenu = true;
				break;

			default:
				// Write content page, set vars and listeners, scroll up, move to any hash
				contentBoxID.innerHTML = marked(request.response, markedOpt);
				initVarAssign(type);
				scrollbarBoxID.scrollTo(0, 0);
				if (urlHash !== null) {
					location.assign(urlHash);
					urlHash = null;
				}
				// Note loaded page to avoid repeat nav OR blank for 404, flag load uccess
				sameLocation = resource;
				if (type === badTimes) sameLocation = "";
				loadedMD = true;
		}
	// Process when file fails to load
	if (requestCheck === reqFail)
		switch (type) {
			case css:
				// CSS can't be loaded, record attempt
				attemptCSS += 1;
				// Halt retries at 3, fake success and continue page processing
				if (attemptCSS === maxAttemptCSS) {
					loadedCSS = true;
					importCSSClean();
					break;
				}
				// Retry loading the stylesheet up to 3 times
				getFile(xhttpCSSFile, resource, css); 
				break;

			default:
				// Content can't be loaded, flag 404 and ensure flags are reset
				loadedCSS = false;
				loadedMD = false;
				linkHandler(badTimes, badTimes);
		}
	// Make sure all resources loaded flags are true
	if (loadedCSS && loadedMD && loadedMenu) {
		// Wait 200ms to ensure content and styles are rendered before disengaging cloak
		setTimeout(contentVisibility(contentShow), contentUncloakTime);
		// Return content and CSS resource flags and attempts to defaults for next cycle
		attemptCSS = 0;
		loadedCSS = false;
		loadedMD = false;
	}
}


// ********************\
// * CSS Handlers
// ********************/
/**
 * @description Removes page-unique stylesheet so new one can be applied.
 */
function importCSSClean() {
	if (head.lastElementChild.type === txtcss) head.removeChild(head.lastElementChild);
}
/**
 * @description Constructs page-unique stylesheet link element and appends to `<head>`.
 *
 * @param {*} resource - File name of the stylesheet to link. Must be indentical to content file.
 */
function importCSS(resource) {
	if (resource !== home) {
		let element = document.createElement(link);
		element.type = txtcss;
		element.rel = stylesheet;
		element.href = dirCSS + resource + extCSS;
		head.appendChild(element);
	}
}
/**
 * @description Visibility toggling for content swaps. Prevents page flicker.
 *
 * @param {*} toggle - Flag to designate running show or hide actions. 
 *	Also contains opacity value to set.
 * 
 * - **contentHide** - `opacity: 0;` 
 * - **contentShow** - `opacity: 100;`
 */
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
		setTimeout(function () {
			loadingID.style.display = displayNone;
		}, contentCloakTime);
	}
}


// ********************\
// * Menu Control
// ********************/
/**
 * @description Applies `will-change` styles on `mouseenter` to elements that will animate.
 * - Currently only used for menu drawer.
 */
function elementWillChange() {
	if ('beginElement' in menuToggleIconFadeInID) menuToggleIconFadeInID.beginElement();
	menuToggleIconID.style.willChange = transform;
	sideMenuID.style.willChange = width;
}
/**
 * @description Removes `will-change` styles on `mouseleave` from elements that would have animated.
 * - Currently only used for menu drawer.
 */
function elementWillChangeFinish() {
	if ('beginElement' in menuToggleIconFadeOutID) menuToggleIconFadeOutID.beginElement();
	// Removing will-change after 1 second
	setTimeout(function () {
		menuToggleIconID.style.willChange = auto;
		sideMenuID.style.willChange = auto;
	}, menuChangeTimeout);
}
/**
 * @description Manages toggle listener swap and menu block width for closing the menu drawer.
 */
function menuClose() {
	menuToggleID.removeEventListener(click, menuClose);
	menuToggleID.addEventListener(click, menuOpen);
	sideMenuID.style.width = sideMenuHide;
	// Drawer animates over 400ms, flip arrow at 450ms
	setTimeout(function () {
		menuToggleIconID.style.transform = menuToggleLeft;
	}, menuSlideFlip);
}
/**
 * @description Manages toggle listener swap and menu block width for opening the menu drawer.
 */
function menuOpen() {
	menuToggleID.removeEventListener(click, menuOpen);
	menuToggleID.addEventListener(click, menuClose);
	sideMenuID.style.width = sideMenuShow;
	// Drawer animates over 400ms, flip arrow at 450ms
	setTimeout(function () {
		menuToggleIconID.style.transform = menuToggleRight;
	}, menuSlideFlip);
}


// ********************\
// * Initializations
// ********************/
/**
 * @description Checks root document to ensure DOM content has been loaded before beginning operations.
 *
 * @param {*} caller - Function to run when DOM status conditions are met.
 * 
 * @returns `document.readyState !== loading` - Immediately begins operations.
 * @returns `document.addEventListener` - DOM not ready yet, so apply listener for required status.
 */
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
/**
 * @description Function to run when DOM content is ready, repairing malformed URLs 
 *	and initializing page operations.
 */
readyUtil(function () {
	// Analyze URL and repair if necessary
	initVarAssign(entry);
	if (urlFixed !== urlRaw.toString()) return window.location.assign(urlFixed);

	// Start CSS variable polyfill
	if (cssVars) cssVars(cssVarsOpt);

	// Init vars and listeners
	initVarAssign(home);

	// Load menu
	linkHandler(menuParam, menu);

	// Load pre-designated destination
	if (urlParam !== null) return linkHandler(urlParam, reentry);

	// Load default destination
	return linkHandler(homeParam, entry);
});
/**
 * @description Function that triggers on window history changes, checking 
 *	current `window.location` and triggering appropriate content load.
 */
window.onpopstate = function () {
	// Analyze URL
	initVarAssign(entry);

	// Build input
	let urlHistory = "";
	if (urlParam !== null) urlHistory += urlParam;
	if (urlHash !== null) urlHistory += urlHash;

	// Load destination without modifying browser history
	linkHandler(urlHistory, null);
};
