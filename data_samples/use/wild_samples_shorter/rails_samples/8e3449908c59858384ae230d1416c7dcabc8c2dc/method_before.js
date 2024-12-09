import { isCrossDomain } from "../utils/ajax"
import * as csrf from "../utils/csrf"
import { stopEverything } from "../utils/event"

// Handles "data-method" on links such as:
// <a href="/users/5" data-method="delete" rel="nofollow" data-confirm="Are you sure?">Delete</a>
const handleMethodWithRails = (rails) => function(e) {
  const method = link.getAttribute("data-method")
  if (!method) { return }

  const href = rails.href(link)
  const csrfToken = csrf.csrfToken()
  const csrfParam = csrf.csrfParam()
  const form = document.createElement("form")