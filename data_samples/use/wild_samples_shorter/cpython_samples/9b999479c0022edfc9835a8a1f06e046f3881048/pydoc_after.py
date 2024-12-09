%s</head><body bgcolor="#f0f0f8">%s<div style="clear:both;padding-top:.5em;">%s</div>
</body></html>''' % (title, css_link, html_navbar(), contents)


    html = _HTMLDoc()

    def html_navbar():
            'key = %s' % key, '#ffffff', '#ee77aa', '<br>'.join(results))
        return 'Search Results', contents

    def html_topics():
        """Index of topic texts available."""

        def bltinlink(name):
                op, _, url = url.partition('=')
                if op == "search?key":
                    title, content = html_search(url)
                elif op == "topic?key":
                    # try topics first, then objects.
                    try:
                        title, content = html_topicpage(url)