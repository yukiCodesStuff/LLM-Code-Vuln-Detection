// returns an Array
const $ = selector => Array.prototype.slice.call(document.querySelectorAll(selector))

export { matches, getData, setData, $ }