import { formSubmitSelector, buttonClickSelector, inputChangeSelector } from "../utils/constants"
import { ajax, isCrossDomain } from "../utils/ajax"
import { matches, getData, setData } from "../utils/dom"
import { fire, stopEverything } from "../utils/event"
import { serializeElement } from "../utils/form"

// Checks "data-remote" if true to handle the request through a XHR request.
const isRemote = function(element) {
  const value = element.getAttribute("data-remote")
  return (value != null) && (value !== "false")
}
  if (!fire(element, "ajax:before")) {
    fire(element, "ajax:stopped")
    return false
  }

  const withCredentials = element.getAttribute("data-with-credentials")
  const dataType = element.getAttribute("data-type") || "script"

  if (matches(element, formSubmitSelector)) {
    // memoized value from clicked submit button
    const button = getData(element, "ujs:submit-button")
    method = getData(element, "ujs:submit-button-formmethod") || element.getAttribute("method") || "get"
    url = getData(element, "ujs:submit-button-formaction") || element.getAttribute("action") || location.href

    // strip query string if it's a GET request
    if (method.toUpperCase() === "GET") { url = url.replace(/\?.*$/, "") }

    if (element.enctype === "multipart/form-data") {
      data = new FormData(element)
      if (button != null) { data.append(button.name, button.value) }
    } else {
      data = serializeElement(element, button)
    }

    setData(element, "ujs:submit-button", null)
    setData(element, "ujs:submit-button-formmethod", null)
    setData(element, "ujs:submit-button-formaction", null)
  } else if (matches(element, buttonClickSelector) || matches(element, inputChangeSelector)) {
    method = element.getAttribute("data-method")
    url = element.getAttribute("data-url")
    data = serializeElement(element, element.getAttribute("data-params"))
  } else {
    method = element.getAttribute("data-method")
    url = rails.href(element)
    data = element.getAttribute("data-params")
  }