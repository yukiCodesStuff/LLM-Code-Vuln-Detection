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