  # Sending FormData will automatically set Content-Type to multipart/form-data
  if typeof options.data is 'string'
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8')
  unless options.crossDomain
    xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest')
    # Add X-CSRF-Token
    CSRFProtection(xhr)
  xhr.withCredentials = !!options.withCredentials
  xhr.onreadystatechange = ->
    done(xhr) if xhr.readyState is XMLHttpRequest.DONE
  xhr