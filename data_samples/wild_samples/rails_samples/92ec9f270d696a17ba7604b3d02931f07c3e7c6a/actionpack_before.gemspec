  s.metadata = {
    "bug_tracker_uri"   => "https://github.com/rails/rails/issues",
    "changelog_uri"     => "https://github.com/rails/rails/blob/v#{version}/actionpack/CHANGELOG.md",
    "documentation_uri" => "https://api.rubyonrails.org/v#{version}/",
    "mailing_list_uri"  => "https://groups.google.com/forum/#!forum/rubyonrails-talk",
    "source_code_uri"   => "https://github.com/rails/rails/tree/v#{version}/actionpack",
  }

  # NOTE: Please read our dependency guidelines before updating versions:
  # https://edgeguides.rubyonrails.org/security.html#dependency-management-and-cves

  s.add_dependency "activesupport", version

  s.add_dependency "rack",      "~> 2.0"
  s.add_dependency "rack-test", ">= 0.6.3"
  s.add_dependency "rails-html-sanitizer", "~> 1.0", ">= 1.2.0"
  s.add_dependency "rails-dom-testing", "~> 2.0"
  s.add_dependency "actionview", version

  s.add_development_dependency "activemodel", version
end