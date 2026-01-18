# Converts > [!TYPE] blocks into HTML alert boxes
Jekyll::Hooks.register [:pages, :documents], :pre_render do |doc|
	next unless doc.extname =~ /\.md|\.markdown|\.mkd|\.mkdn|\.mkdown/

	content = doc.content

	# Match only contiguous > lines immediately after [!TYPE]
	alert_regex = /^> \[!(\w+)\][^\n]*\n((?:>[^\n]*\n)*)/m

	content.gsub!(alert_regex) do
		type = Regexp.last_match(1).downcase
		body = Regexp.last_match(2)

		# Strip leading "> " from each line
		body_lines = body.lines.map { |line| line.sub(/^> ?/, '') }

		# Remove leading/trailing blank lines
		body_lines.shift while body_lines.first&.strip == ""
		body_lines.pop   while body_lines.last&.strip == ""

		html_body = Kramdown::Document.new(body_lines.join, input: 'GFM').to_html

		<<~HTML
		<div class="alert alert-#{type}">
			<div class="alert-title">#{type.upcase}</div>
			<div class="alert-content">
				#{html_body}
			</div>
		</div>
		HTML
	end

	doc.content = content
end

