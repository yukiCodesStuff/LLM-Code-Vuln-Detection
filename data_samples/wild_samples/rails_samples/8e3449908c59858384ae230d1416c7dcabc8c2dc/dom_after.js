const setData = function(element, key, value) {
  if (!element[EXPANDO]) { element[EXPANDO] = {} }
  return element[EXPANDO][key] = value
}

// a wrapper for document.querySelectorAll
// returns an Array
const $ = selector => Array.prototype.slice.call(document.querySelectorAll(selector))

const isContentEditable = function(element) {
  var isEditable = false
  do {
    if(element.isContentEditable) {
      isEditable = true
      break
    }

    element = element.parentElement
  } while(element)

  return isEditable
}

export { matches, getData, setData, $, isContentEditable }