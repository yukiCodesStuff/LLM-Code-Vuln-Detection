# Domain
<!-- YAML
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/REPLACEME
    description: Any `Promise`s created in VM contexts no longer have a
                 `.domain` property. Their handlers are still executed in the
                 proper domain, however, and `Promise`s created in the main
                 context still possess a `.domain` property.