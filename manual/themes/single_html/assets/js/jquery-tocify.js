
(function(tocify) {

    // ECMAScript 5 Strict Mode
    "use strict";

    // Calls the second IIFE and locally passes in the global jQuery, window, and document objects
    tocify(window.jQuery, window, document);

}

(function($, window, document, undefined) {

    // ECMAScript 5 Strict Mode
    "use strict";

    var tocClassName = "tocify",
        tocClass = "." + tocClassName,
        tocFocusClassName = "tocify-focus",
        tocHoverClassName = "tocify-hover",
        hideTocClassName = "tocify-hide",
        hideTocClass = "." + hideTocClassName,
        headerClassName = "tocify-header",
        headerClass = "." + headerClassName,
        subheaderClassName = "tocify-subheader",
        subheaderClass = "." + subheaderClassName,
        itemClassName = "tocify-item",
        itemClass = "." + itemClassName,
        extendPageClassName = "tocify-extend-page",
        extendPageClass = "." + extendPageClassName;

    // Calling the jQueryUI Widget Factory Method
    $.widget("toc.tocify", {

        version: "1.9.0",

        // These options will be used as defaults
        options: {

            context: "body",
            ignoreSelector: null,
            selectors: "h1, h2, h3",
            showAndHide: true,
            showEffect: "slideDown",
            showEffectSpeed: "medium",
            hideEffect: "slideUp",
            hideEffectSpeed: "medium",
            smoothScroll: true,
            smoothScrollSpeed: "medium",
            scrollTo: 0,
            showAndHideOnScroll: true,
            highlightOnScroll: true,
            highlightOffset: 40,
            theme: "bootstrap",
            extendPage: true,
            extendPageOffset: 100,
            history: true,
            scrollHistory: false,
            hashGenerator: "compact",
            highlightDefault: true
        },


        // _Create
        _create: function() {
            var self = this;
            self.extendPageScroll = true;

            // Internal array that keeps track of all TOC items (Helps to recognize if there are duplicate TOC item strings)
            self.items = [];

            // Generates the HTML for the dynamic table of contents
            self._generateToc();

            // Adds CSS classes to the newly generated table of contents HTML
            self._addCSSClasses();

            self.webkit = (function() {
                for(var prop in window) {
                    if(prop) {
                        if(prop.toLowerCase().indexOf("webkit") !== -1) {
                            return true;
                        }

                    }

                }

                return false;

            }());

           // Adds jQuery event handlers to the newly generated table of contents
            self._setEventHandlers();

            // Binding to the Window load event to make sure the correct scrollTop is calculated
            $(window).load(function() {

                // Sets the active TOC item
                self._setActiveElement(true);

                // Once all animations on the page are complete, this callback function will be called
                $("html, body").promise().done(function() {

                    setTimeout(function() {

                        self.extendPageScroll = false;

                    },0);

                });

            });

        },

        // _generateToc
        _generateToc: function() {

            // Stores the plugin context in the self variable
            var self = this,
                firstElem,
                ul,
                ignoreSelector = self.options.ignoreSelector;
             if(this.options.selectors.indexOf(",") !== -1) {
                 // Grabs the first selector from the string
                 firstElem = $(this.options.context).find(this.options.selectors.replace(/ /g,"").substr(0, this.options.selectors.indexOf(",")));
             }
             else {
                 firstElem = $(this.options.context).find(this.options.selectors.replace(/ /g,""));
             }

            if(!firstElem.length) {
                self.element.addClass(hideTocClassName);
                return;
            }

            self.element.addClass(tocClassName);

            // Loops through each top level selector
            firstElem.each(function(index) {

                if($(this).is(ignoreSelector)) {
                    return;
                }

                ul = $("<ul/>", {
                    "id": headerClassName + index,
                    "class": headerClassName
                }).

                // Appends a top level list item HTML element to the previously created HTML header
                append(self._nestElements($(this), index));

                // Add the created unordered list element to the HTML element calling the plugin
                self.element.append(ul);

                // Finds all of the HTML tags between the header and subheader elements
                $(this).nextUntil(this.nodeName.toLowerCase()).each(function() {

                    // If there are no nested subheader elemements
                    if($(this).find(self.options.selectors).length === 0) {

                        // Loops through all of the subheader elements
                        $(this).filter(self.options.selectors).each(function() {

                            //If the element matches the ignoreSelector then we skip it
                            if($(this).is(ignoreSelector)) {
                                return;
                            }

                            self._appendSubheaders.call(this, self, ul);

                        });

                    }

                    // If there are nested subheader elements
                    else {

                        // Loops through all of the subheader elements
                        $(this).find(self.options.selectors).each(function() {

                            //If the element matches the ignoreSelector then we skip it
                            if($(this).is(ignoreSelector)) {
                                return;
                            }

                            self._appendSubheaders.call(this, self, ul);

                        });

                    }

                });

            });

        },

      _setActiveElement: function(pageload) {

            var self = this,

                hash = window.location.hash.substring(1),

                elem = self.element.find('li[data-unique="' + hash + '"]');

            if(hash.length) {

                // Removes highlighting from all of the list item's
                self.element.find("." + self.focusClass).removeClass(self.focusClass);

                // Highlights the current list item that was clicked
                elem.addClass(self.focusClass);

                // If the showAndHide option is true
                if(self.options.showAndHide) {

                    // Triggers the click event on the currently focused TOC item
                    elem.click();

                }

            }

            else {

                // Removes highlighting from all of the list item's
                self.element.find("." + self.focusClass).removeClass(self.focusClass);

                if(!hash.length && pageload && self.options.highlightDefault) {

                    // Highlights the first TOC item if no other items are highlighted
                    self.element.find(itemClass).first().addClass(self.focusClass);

                }

            }

            return self;

        },

        //      Create the table of contents list by appending nested list items
        _nestElements: function(self, index) {

            var arr, item, hashValue;

            arr = $.grep(this.items, function (item) {

                return item === self.text();

            });

            // If there is already a duplicate TOC item
            if(arr.length) {

                // Adds the current TOC item text and index to the internal array
                this.items.push(self.text() + index);

            }

            // If there not a duplicate TOC item
            else {

                // Adds the current TOC item text to the internal array
                this.items.push(self.text());

            }

            hashValue = this._generateHashValue(arr, self, index);

            // Appends a list item HTML element to the last unordered list HTML element
            item = $("<li/>", {

                // Sets a common class name to the list item
                "class": itemClassName,

                "data-unique": hashValue

            }).append($("<a/>", {

                "text": self.text()

            }));

            // Adds an HTML anchor tag before the currently traversed HTML element
            self.before($("<div/>", {

                // Sets a name attribute on the anchor tag to the text of the currently traversed HTML element (also making sure that all whitespace is replaced with an underscore)
                "name": hashValue,

                "data-unique": hashValue

            }));

            return item;

        },

       //      Generates the hash value that will be used to refer to each item.
        _generateHashValue: function(arr, self, index) {

            var hashValue = "",
                hashGeneratorOption = this.options.hashGenerator;

            if (hashGeneratorOption === "pretty") {

                hashValue = self.text().toLowerCase().replace(/\s/g, "-");

                // fix double hyphens
                while (hashValue.indexOf("--") > -1) {
                    hashValue = hashValue.replace(/--/g, "-");
                }

                // fix colon-space instances
                while (hashValue.indexOf(":-") > -1) {
                    hashValue = hashValue.replace(/:-/g, "-");
                }

            } else if (typeof hashGeneratorOption === "function") {

                hashValue = hashGeneratorOption(self.text(), self);

            } else {

                hashValue = self.text().replace(/\s/g, "");

            }

            if (arr.length) { hashValue += ""+index; }
           return hashValue;

        },

        //      Create the table of contents list by appending subheader elements

        _appendSubheaders: function(self, ul) {

            // The current element index
            var index = $(this).index(self.options.selectors),

                // Finds the previous header DOM element
                previousHeader = $(self.options.selectors).eq(index - 1),

                currentTagName = +$(this).prop("tagName").charAt(1),

                previousTagName = +previousHeader.prop("tagName").charAt(1),

                lastSubheader;

            // If the current header DOM element is smaller than the previous header DOM element or the first subheader
            if(currentTagName < previousTagName) {

                // Selects the last unordered list HTML found within the HTML element calling the plugin
                self.element.find(subheaderClass + "[data-tag=" + currentTagName + "]").last().append(self._nestElements($(this), index));

            }

             else if(currentTagName === previousTagName) {

                ul.find(itemClass).last().after(self._nestElements($(this), index));

            }

            else {

                // Selects the last unordered list HTML found within the HTML element calling the plugin
                ul.find(itemClass).last().

                // Appends an unorderedList HTML element to the dynamic `unorderedList` variable and sets a common class name
                after($("<ul/>", {

                    "class": subheaderClassName,

                    "data-tag": currentTagName

                })).next(subheaderClass).

                // Appends a list item HTML element to the last unordered list HTML element found within the HTML element calling the plugin
                append(self._nestElements($(this), index));
            }

        },

        //      Adds jQuery event handlers to the newly generated table of contents
        _setEventHandlers: function() {

            var self = this,
                $self,
                duration;
            this.element.on("click.tocify", "li", function(event) {
                if(self.options.history) {
                    window.location.hash = $(this).attr("data-unique");
                }

                // Removes highlighting from all of the list item's
                self.element.find("." + self.focusClass).removeClass(self.focusClass);

                // Highlights the current list item that was clicked
                $(this).addClass(self.focusClass);

                // If the showAndHide option is true
                if(self.options.showAndHide) {

                    var elem = $('li[data-unique="' + $(this).attr("data-unique") + '"]');

                    self._triggerShow(elem);
                }
                self._scrollTo($(this));
            });

            // Mouseenter and Mouseleave event handlers for the list item's within the HTML element calling the plugin
            this.element.find("li").on({

                // Mouseenter event handler
                "mouseenter.tocify": function() {

                    // Adds a hover CSS class to the current list item
                    $(this).addClass(self.hoverClass);

                    // Makes sure the cursor is set to the pointer icon
                    $(this).css("cursor", "pointer");

                },

                // Mouseleave event handler
                "mouseleave.tocify": function() {

                    if(self.options.theme !== "bootstrap") {

                        // Removes the hover CSS class from the current list item
                        $(this).removeClass(self.hoverClass);

                    }

                }
            });

            // only attach handler if needed (expensive in IE)
            if (self.options.extendPage || self.options.highlightOnScroll || self.options.scrollHistory || self.options.showAndHideOnScroll)
            {
            // Window scroll event handler
                $(window).on("scroll.tocify", function() {

                    // Once all animations on the page are complete, this callback function will be called
                    $("html, body").promise().done(function() {

                        // Stores how far the user has scrolled
                        var winScrollTop = $(window).scrollTop(),

                            // Stores the height of the window
                            winHeight = $(window).height(),

                            // Stores the height of the document
                            docHeight = $(document).height(),

                            scrollHeight = $("body")[0].scrollHeight,

                            // Instantiates a variable that will be used to hold a selected HTML element
                            elem,

                            lastElem,

                            lastElemOffset,

                            currentElem;

                        if(self.options.extendPage) {

                            // If the user has scrolled to the bottom of the page and the last toc item is not focused
                            if((self.webkit && winScrollTop >= scrollHeight - winHeight - self.options.extendPageOffset) || (!self.webkit && winHeight + winScrollTop > docHeight - self.options.extendPageOffset)) {

                                if(!$(extendPageClass).length) {

                                    lastElem = $('div[data-unique="' + $(itemClass).last().attr("data-unique") + '"]');

                                    if(!lastElem.length) return;

                                    // Gets the top offset of the page header that is linked to the last toc item
                                    lastElemOffset = lastElem.offset().top;

                                    // Appends a div to the bottom of the page and sets the height to the difference of the window scrollTop and the last element's position top offset
                                    $(self.options.context).append($("<div />", {

                                        "class": extendPageClassName,

                                        "height": Math.abs(lastElemOffset - winScrollTop) + "px",

                                        "data-unique": extendPageClassName

                                    }));

                                    if(self.extendPageScroll) {

                                        currentElem = self.element.find('li.active');

                                        self._scrollTo($('div[data-unique="' + currentElem.attr("data-unique") + '"]'));

                                    }

                                }

                            }

                        }

                        // The zero timeout ensures the following code is run after the scroll events
                        setTimeout(function() {

                            // Stores the distance to the closest anchor
                            var closestAnchorDistance = null,

                                // Stores the index of the closest anchor
                                closestAnchorIdx = null,

                                // Keeps a reference to all anchors
                                anchors = $(self.options.context).find("div[data-unique]"),

                                anchorText;

                            // Determines the index of the closest anchor
                            anchors.each(function(idx) {
                                var distance = Math.abs(($(this).next().length ? $(this).next() : $(this)).offset().top - winScrollTop - self.options.highlightOffset);
                                if (closestAnchorDistance == null || distance < closestAnchorDistance) {
                                    closestAnchorDistance = distance;
                                    closestAnchorIdx = idx;
                                } else {
                                    return false;
                                }
                            });

                            anchorText = $(anchors[closestAnchorIdx]).attr("data-unique");

                            // Stores the list item HTML element that corresponds to the currently traversed anchor tag
                            elem = $('li[data-unique="' + anchorText + '"]');

                            // If the `highlightOnScroll` option is true and a next element is found
                            if(self.options.highlightOnScroll && elem.length) {

                                // Removes highlighting from all of the list item's
                                self.element.find("." + self.focusClass).removeClass(self.focusClass);

                                // Highlights the corresponding list item
                                elem.addClass(self.focusClass);

                            }

                            if(self.options.scrollHistory) {

                                if(window.location.hash !== "#" + anchorText) {

                                    window.location.replace("#" + anchorText);

                                }
                            }

                            // If the `showAndHideOnScroll` option is true
                            if(self.options.showAndHideOnScroll && self.options.showAndHide) {

                                self._triggerShow(elem, true);

                            }

                        }, 0);

                    });

                });
            }

        },
