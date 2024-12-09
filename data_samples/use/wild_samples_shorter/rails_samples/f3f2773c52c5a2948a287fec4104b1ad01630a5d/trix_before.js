/*
Trix 2.0.7
Copyright © 2023 37signals, LLC
 */
(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
  typeof define === 'function' && define.amd ? define(factory) :
})(this, (function () { 'use strict';

  var name = "trix";
  var version = "2.0.7";
  var description = "A rich text editor for everyday writing";
  var main = "dist/trix.umd.min.js";
  var module = "dist/trix.esm.min.js";
  var style = "dist/trix.css";
  var files = [
  	"dist/*.css",
  	"dist/*.js",
  	"src/{inspector,trix}/*.js"
  ];
  var repository = {
  	type: "git",
    code: {
      tagName: "pre",
      terminal: true,
      text: {
        plaintext: true
      }
    },
      no-useless-escape,
  */
  const normalizeSpaces = string => string.replace(new RegExp("".concat(ZERO_WIDTH_SPACE), "g"), "").replace(new RegExp("".concat(NON_BREAKING_SPACE), "g"), " ");
  const normalizeNewlines = string => string.replace(/\r\n/g, "\n");
  const breakableWhitespacePattern = new RegExp("[^\\S".concat(NON_BREAKING_SPACE, "]"));
  const squishBreakableWhitespace = string => string
  // Replace all breakable whitespace characters with a space
  .replace(new RegExp("".concat(breakableWhitespacePattern.source), "g"), " ")
      }
    }
    createContainerElement(depth) {
      let attributes, className;
      const attributeName = this.attributes[depth];
      const {
        tagName
      } = getBlockConfig(attributeName);
      if (depth === 0 && this.block.isRTL()) {
        attributes = {
          dir: "rtl"
        };
      }
      if (attributeName === "attachmentGallery") {
        const size = this.block.getBlockBreakPosition();
        className = "".concat(css$1.attachmentGallery, " ").concat(css$1.attachmentGallery, "--").concat(size);
      }
      return makeElement({
        tagName,
        className,
        attributes
  class Block extends TrixObject {
    static fromJSON(blockJSON) {
      const text = Text.fromJSON(blockJSON.text);
      return new this(text, blockJSON.attributes);
    }
    constructor(text, attributes) {
      super(...arguments);
      this.text = applyBlockBreakToText(text || new Text());
      this.attributes = attributes || [];
    }
    isEmpty() {
      return this.text.isBlockBreak();
    }
    isEqualTo(block) {
      if (super.isEqualTo(block)) return true;
      return this.text.isEqualTo(block === null || block === void 0 ? void 0 : block.text) && arraysAreEqual(this.attributes, block === null || block === void 0 ? void 0 : block.attributes);
    }
    copyWithText(text) {
      return new Block(text, this.attributes);
    }
    copyWithoutText() {
      return this.copyWithText(null);
    }
    copyWithAttributes(attributes) {
      return new Block(this.text, attributes);
    }
    copyWithoutAttributes() {
      return this.copyWithAttributes(null);
    }
      const attributes = this.attributes.concat(expandAttribute(attribute));
      return this.copyWithAttributes(attributes);
    }
    removeAttribute(attribute) {
      const {
        listAttribute
      } = getBlockConfig(attribute);
    toJSON() {
      return {
        text: this.text,
        attributes: this.attributes
      };
    }

    // BIDI
      const range = this.getRangeOfAttachment(attachment);
      return this.removeAttributeAtRange(attribute, range);
    }
    insertBlockBreakAtRange(range) {
      let blocks;
      range = normalizeRange(range);
      const [startPosition] = range;
    return attributes;
  };

  const DEFAULT_ALLOWED_ATTRIBUTES = "style href src width height class".split(" ");
  const DEFAULT_FORBIDDEN_PROTOCOLS = "javascript:".split(" ");
  const DEFAULT_FORBIDDEN_ELEMENTS = "script iframe form".split(" ");
  class HTMLSanitizer extends BasicObject {
    static sanitize(html, options) {
      const sanitizer = new this(html, options);
      sanitizer.sanitize();
  };
  const blockForAttributes = function () {
    let attributes = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
    const text = [];
    return {
      text,
      attributes
    };
  };
  const parseTrixDataAttribute = (element, name) => {
    try {
      return JSON.parse(element.getAttribute("data-trix-".concat(name)));
    } catch (error) {
      return {};
    }
  };
      } else if (element === this.containerElement || this.isBlockElement(element)) {
        var _this$currentBlock;
        const attributes = this.getBlockAttributes(element);
        if (!arraysAreEqual(attributes, (_this$currentBlock = this.currentBlock) === null || _this$currentBlock === void 0 ? void 0 : _this$currentBlock.attributes)) {
          this.currentBlock = this.appendBlockForAttributesWithElement(attributes, element);
          this.currentBlockElement = element;
        }
      }
    }
      if (elementIsBlockElement && !this.isBlockElement(element.firstChild)) {
        if (!this.isInsignificantTextNode(element.firstChild) || !this.isBlockElement(element.firstElementChild)) {
          const attributes = this.getBlockAttributes(element);
          if (element.firstChild) {
            if (!(currentBlockContainsElement && arraysAreEqual(attributes, this.currentBlock.attributes))) {
              this.currentBlock = this.appendBlockForAttributesWithElement(attributes, element);
              this.currentBlockElement = element;
            } else {
              return this.appendStringWithAttributes("\n");
            }
    // Document construction

    appendBlockForAttributesWithElement(attributes, element) {
      this.blockElements.push(element);
      const block = blockForAttributes(attributes);
      this.blocks.push(block);
      return block;
    }
    appendEmptyBlock() {
      }
      return attributes$1.reverse();
    }
    findBlockElementAncestors(element) {
      const ancestors = [];
      while (element && element !== this.containerElement) {
        const tag = tagName(element);
        return this.notifyDelegateOfCurrentAttributesChange();
      }
    }
    setTextAttribute(attributeName, value) {
      const selectedRange = this.getSelectedRange();
      if (!selectedRange) return;
      const [startPosition, endPosition] = Array.from(selectedRange);
      return ((_this$getBlock = this.getBlock()) === null || _this$getBlock === void 0 ? void 0 : _this$getBlock.getNestingLevel()) > 0;
    }
    canIncreaseNestingLevel() {
      var _getBlockConfig;
      const block = this.getBlock();
      if (!block) return;
      if ((_getBlockConfig = getBlockConfig(block.getLastNestableAttribute())) !== null && _getBlockConfig !== void 0 && _getBlockConfig.listAttribute) {
        const previousBlock = this.getPreviousBlock();
        if (previousBlock) {
          return arrayStartsWith(previousBlock.getListItemAttributes(), block.getListItemAttributes());
        }
      return this.composition.removeCurrentAttribute(name);
    }

    // Nesting level

    canDecreaseNestingLevel() {
      return this.composition.canDecreaseNestingLevel();
      });
    },
    insertReplacementText() {
      return this.insertString(this.event.dataTransfer.getData("text/plain"), {
        updatePosition: false
      });
    },
    insertText() {
      var _this$event$dataTrans;
        return this.toggleDialog(actionName);
      } else {
        var _this$delegate2;
        return (_this$delegate2 = this.delegate) === null || _this$delegate2 === void 0 ? void 0 : _this$delegate2.toolbarDidInvokeAction(actionName);
      }
    }
    didClickAttributeButton(event, element) {
      var _this$delegate3;
        });
      }
    }
    toolbarDidInvokeAction(actionName) {
      return this.invokeAction(actionName);
    }
    toolbarDidToggleAttribute(attributeName) {
      this.recordFormattingUndoEntry(attributeName);
      this.composition.toggleCurrentAttribute(attributeName);
        return !!((_this$actions$actionN = this.actions[actionName]) !== null && _this$actions$actionN !== void 0 && (_this$actions$actionN = _this$actions$actionN.test) !== null && _this$actions$actionN !== void 0 && _this$actions$actionN.call(this));
      }
    }
    invokeAction(actionName) {
      if (this.actionIsExternal(actionName)) {
        return this.notifyEditorElement("action-invoke", {
          actionName
        });
      } else {
        var _this$actions$actionN2;
        return (_this$actions$actionN2 = this.actions[actionName]) === null || _this$actions$actionN2 === void 0 || (_this$actions$actionN2 = _this$actions$actionN2.perform) === null || _this$actions$actionN2 === void 0 ? void 0 : _this$actions$actionN2.call(this);