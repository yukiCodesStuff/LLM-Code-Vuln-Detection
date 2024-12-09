/*
Trix 2.1.1
Copyright © 2024 37signals, LLC
 */
(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
  typeof define === 'function' && define.amd ? define(factory) :
})(this, (function () { 'use strict';

  var name = "trix";
  var version = "2.1.1";
  var description = "A rich text editor for everyday writing";
  var main = "dist/trix.umd.min.js";
  var module = "dist/trix.esm.min.js";
  var style = "dist/trix.css";
  var files = [
  	"dist/*.css",
  	"dist/*.js",
  	"dist/*.map",
  	"src/{inspector,trix}/*.js"
  ];
  var repository = {
  	type: "git",
    code: {
      tagName: "pre",
      terminal: true,
      htmlAttributes: ["language"],
      text: {
        plaintext: true
      }
    },
      no-useless-escape,
  */
  const normalizeSpaces = string => string.replace(new RegExp("".concat(ZERO_WIDTH_SPACE), "g"), "").replace(new RegExp("".concat(NON_BREAKING_SPACE), "g"), " ");
  const normalizeNewlines = string => string.replace(/\r\n?/g, "\n");
  const breakableWhitespacePattern = new RegExp("[^\\S".concat(NON_BREAKING_SPACE, "]"));
  const squishBreakableWhitespace = string => string
  // Replace all breakable whitespace characters with a space
  .replace(new RegExp("".concat(breakableWhitespacePattern.source), "g"), " ")
      }
    }
    createContainerElement(depth) {
      const attributes = {};
      let className;
      const attributeName = this.attributes[depth];
      const {
        tagName,
        htmlAttributes = []
      } = getBlockConfig(attributeName);
      if (depth === 0 && this.block.isRTL()) {
        Object.assign(attributes, {
          dir: "rtl"
        });
      }
      if (attributeName === "attachmentGallery") {
        const size = this.block.getBlockBreakPosition();
        className = "".concat(css$1.attachmentGallery, " ").concat(css$1.attachmentGallery, "--").concat(size);
      }
      Object.entries(this.block.htmlAttributes).forEach(_ref => {
        let [name, value] = _ref;
        if (htmlAttributes.includes(name)) {
          attributes[name] = value;
        }
      });
      return makeElement({
        tagName,
        className,
        attributes
  class Block extends TrixObject {
    static fromJSON(blockJSON) {
      const text = Text.fromJSON(blockJSON.text);
      return new this(text, blockJSON.attributes, blockJSON.htmlAttributes);
    }
    constructor(text, attributes, htmlAttributes) {
      super(...arguments);
      this.text = applyBlockBreakToText(text || new Text());
      this.attributes = attributes || [];
      this.htmlAttributes = htmlAttributes || {};
    }
    isEmpty() {
      return this.text.isBlockBreak();
    }
    isEqualTo(block) {
      if (super.isEqualTo(block)) return true;
      return this.text.isEqualTo(block === null || block === void 0 ? void 0 : block.text) && arraysAreEqual(this.attributes, block === null || block === void 0 ? void 0 : block.attributes) && objectsAreEqual(this.htmlAttributes, block === null || block === void 0 ? void 0 : block.htmlAttributes);
    }
    copyWithText(text) {
      return new Block(text, this.attributes, this.htmlAttributes);
    }
    copyWithoutText() {
      return this.copyWithText(null);
    }
    copyWithAttributes(attributes) {
      return new Block(this.text, attributes, this.htmlAttributes);
    }
    copyWithoutAttributes() {
      return this.copyWithAttributes(null);
    }
      const attributes = this.attributes.concat(expandAttribute(attribute));
      return this.copyWithAttributes(attributes);
    }
    addHTMLAttribute(attribute, value) {
      const htmlAttributes = Object.assign({}, this.htmlAttributes, {
        [attribute]: value
      });
      return new Block(this.text, this.attributes, htmlAttributes);
    }
    removeAttribute(attribute) {
      const {
        listAttribute
      } = getBlockConfig(attribute);
    toJSON() {
      return {
        text: this.text,
        attributes: this.attributes,
        htmlAttributes: this.htmlAttributes
      };
    }

    // BIDI
      const range = this.getRangeOfAttachment(attachment);
      return this.removeAttributeAtRange(attribute, range);
    }
    setHTMLAttributeAtPosition(position, name, value) {
      const block = this.getBlockAtPosition(position);
      const updatedBlock = block.addHTMLAttribute(name, value);
      return this.replaceBlock(block, updatedBlock);
    }
    insertBlockBreakAtRange(range) {
      let blocks;
      range = normalizeRange(range);
      const [startPosition] = range;
    return attributes;
  };

  const DEFAULT_ALLOWED_ATTRIBUTES = "style href src width height language class".split(" ");
  const DEFAULT_FORBIDDEN_PROTOCOLS = "javascript:".split(" ");
  const DEFAULT_FORBIDDEN_ELEMENTS = "script iframe form noscript".split(" ");
  class HTMLSanitizer extends BasicObject {
    static sanitize(html, options) {
      const sanitizer = new this(html, options);
      sanitizer.sanitize();
  };
  const blockForAttributes = function () {
    let attributes = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
    let htmlAttributes = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
    const text = [];
    return {
      text,
      attributes,
      htmlAttributes
    };
  };
  const parseTrixDataAttribute = (element, name) => {
    try {
      const data = JSON.parse(element.getAttribute("data-trix-".concat(name)));
      if (data.contentType === "text/html" && data.content) {
        data.content = HTMLSanitizer.sanitize(data.content).getHTML();
      }
      return data;
    } catch (error) {
      return {};
    }
  };
      } else if (element === this.containerElement || this.isBlockElement(element)) {
        var _this$currentBlock;
        const attributes = this.getBlockAttributes(element);
        const htmlAttributes = this.getBlockHTMLAttributes(element);
        if (!arraysAreEqual(attributes, (_this$currentBlock = this.currentBlock) === null || _this$currentBlock === void 0 ? void 0 : _this$currentBlock.attributes)) {
          this.currentBlock = this.appendBlockForAttributesWithElement(attributes, element, htmlAttributes);
          this.currentBlockElement = element;
        }
      }
    }
      if (elementIsBlockElement && !this.isBlockElement(element.firstChild)) {
        if (!this.isInsignificantTextNode(element.firstChild) || !this.isBlockElement(element.firstElementChild)) {
          const attributes = this.getBlockAttributes(element);
          const htmlAttributes = this.getBlockHTMLAttributes(element);
          if (element.firstChild) {
            if (!(currentBlockContainsElement && arraysAreEqual(attributes, this.currentBlock.attributes))) {
              this.currentBlock = this.appendBlockForAttributesWithElement(attributes, element, htmlAttributes);
              this.currentBlockElement = element;
            } else {
              return this.appendStringWithAttributes("\n");
            }
    // Document construction

    appendBlockForAttributesWithElement(attributes, element) {
      let htmlAttributes = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};
      this.blockElements.push(element);
      const block = blockForAttributes(attributes, htmlAttributes);
      this.blocks.push(block);
      return block;
    }
    appendEmptyBlock() {
      }
      return attributes$1.reverse();
    }
    getBlockHTMLAttributes(element) {
      const attributes$1 = {};
      const blockConfig = Object.values(attributes).find(settings => settings.tagName === tagName(element));
      const allowedAttributes = (blockConfig === null || blockConfig === void 0 ? void 0 : blockConfig.htmlAttributes) || [];
      allowedAttributes.forEach(attribute => {
        if (element.hasAttribute(attribute)) {
          attributes$1[attribute] = element.getAttribute(attribute);
        }
      });
      return attributes$1;
    }
    findBlockElementAncestors(element) {
      const ancestors = [];
      while (element && element !== this.containerElement) {
        const tag = tagName(element);
        return this.notifyDelegateOfCurrentAttributesChange();
      }
    }
    setHTMLAtributeAtPosition(position, attributeName, value) {
      var _getBlockConfig;
      const block = this.document.getBlockAtPosition(position);
      const allowedHTMLAttributes = (_getBlockConfig = getBlockConfig(block.getLastAttribute())) === null || _getBlockConfig === void 0 ? void 0 : _getBlockConfig.htmlAttributes;
      if (block && allowedHTMLAttributes !== null && allowedHTMLAttributes !== void 0 && allowedHTMLAttributes.includes(attributeName)) {
        const newDocument = this.document.setHTMLAttributeAtPosition(position, attributeName, value);
        this.setDocument(newDocument);
      }
    }
    setTextAttribute(attributeName, value) {
      const selectedRange = this.getSelectedRange();
      if (!selectedRange) return;
      const [startPosition, endPosition] = Array.from(selectedRange);
      return ((_this$getBlock = this.getBlock()) === null || _this$getBlock === void 0 ? void 0 : _this$getBlock.getNestingLevel()) > 0;
    }
    canIncreaseNestingLevel() {
      var _getBlockConfig2;
      const block = this.getBlock();
      if (!block) return;
      if ((_getBlockConfig2 = getBlockConfig(block.getLastNestableAttribute())) !== null && _getBlockConfig2 !== void 0 && _getBlockConfig2.listAttribute) {
        const previousBlock = this.getPreviousBlock();
        if (previousBlock) {
          return arrayStartsWith(previousBlock.getListItemAttributes(), block.getListItemAttributes());
        }
      return this.composition.removeCurrentAttribute(name);
    }

    // HTML attributes
    setHTMLAtributeAtPosition(position, name, value) {
      this.composition.setHTMLAtributeAtPosition(position, name, value);
    }

    // Nesting level

    canDecreaseNestingLevel() {
      return this.composition.canDecreaseNestingLevel();
      });
    },
    insertReplacementText() {
      const replacement = this.event.dataTransfer.getData("text/plain");
      const domRange = this.event.getTargetRanges()[0];
      this.withTargetDOMRange(domRange, () => {
        this.insertString(replacement, {
          updatePosition: false
        });
      });
    },
    insertText() {
      var _this$event$dataTrans;
        return this.toggleDialog(actionName);
      } else {
        var _this$delegate2;
        return (_this$delegate2 = this.delegate) === null || _this$delegate2 === void 0 ? void 0 : _this$delegate2.toolbarDidInvokeAction(actionName, element);
      }
    }
    didClickAttributeButton(event, element) {
      var _this$delegate3;
        });
      }
    }
    toolbarDidInvokeAction(actionName, invokingElement) {
      return this.invokeAction(actionName, invokingElement);
    }
    toolbarDidToggleAttribute(attributeName) {
      this.recordFormattingUndoEntry(attributeName);
      this.composition.toggleCurrentAttribute(attributeName);
        return !!((_this$actions$actionN = this.actions[actionName]) !== null && _this$actions$actionN !== void 0 && (_this$actions$actionN = _this$actions$actionN.test) !== null && _this$actions$actionN !== void 0 && _this$actions$actionN.call(this));
      }
    }
    invokeAction(actionName, invokingElement) {
      if (this.actionIsExternal(actionName)) {
        return this.notifyEditorElement("action-invoke", {
          actionName,
          invokingElement
        });
      } else {
        var _this$actions$actionN2;
        return (_this$actions$actionN2 = this.actions[actionName]) === null || _this$actions$actionN2 === void 0 || (_this$actions$actionN2 = _this$actions$actionN2.perform) === null || _this$actions$actionN2 === void 0 ? void 0 : _this$actions$actionN2.call(this);