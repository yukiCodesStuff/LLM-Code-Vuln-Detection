const setData = function(element, key, value) {
  if (!element[EXPANDO]) { element[EXPANDO] = {} }
  return element[EXPANDO][key] = value
}

// a wrapper for document.querySelectorAll
// returns an Array
const $ = selector => Array.prototype.slice.call(document.querySelectorAll(selector))

export { matches, getData, setData, $ }