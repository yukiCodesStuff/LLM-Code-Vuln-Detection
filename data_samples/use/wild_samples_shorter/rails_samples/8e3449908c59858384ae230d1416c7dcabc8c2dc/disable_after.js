import { matches, getData, setData } from "../utils/dom"
import { stopEverything } from "../utils/event"
import { formElements } from "../utils/form"
import { isContentEditable } from "../utils/dom"

const handleDisabledElement = function(e) {
  const element = this
  if (element.disabled) { stopEverything(e) }
    element = e
  }

  if (isContentEditable(element)) {
    return
  }

  if (matches(element, linkDisableSelector)) {
    return enableLinkElement(element)
  } else if (matches(element, buttonDisableSelector) || matches(element, formEnableSelector)) {
    return enableFormElement(element)
// Unified function to disable an element (link, button and form)
const disableElement = (e) => {
  const element = e instanceof Event ? e.target : e

  if (isContentEditable(element)) {
    return
  }

  if (matches(element, linkDisableSelector)) {
    return disableLinkElement(element)
  } else if (matches(element, buttonDisableSelector) || matches(element, formDisableSelector)) {
    return disableFormElement(element)