# frozen_string_literal: true

require 'rouge'

module Rouge
	module Lexers
		class Weaver < RegexLexer
			title "Weaver"
			desc "Weaver / Loom language"
			tag 'weaver'
			filenames '*.wv', '*.weaver'

			state :root do
				# Line comments
				rule %r{//.*$}, Comment::Single

				# Block comments
				rule %r{/\*.*?\*/}m, Comment::Multiline

				# Keywords: features
				rule %r/\b(func|struct|circ|proto)\b/, Keyword::Declaration

				# Include
				rule %r/\bimport\b/, Keyword::Namespace

				# Control flow
				rule %r/\b(region|while|await|if|and|or|xor|var)\b/, Keyword

				# Types
				rule %r/\b(chan|fixed|ufixed|bool)\b/, Keyword::Type

				# TODO markers (inside comments is fine; Rouge does not do containment)
				rule %r/\b(TODO|FIXME|NOTE|DESIGN)\b/, Comment::Special

				# Numbers
				rule %r/[+-]?\d+(\.\d+)?(e[+-]?\d+)?/, Num

				# Braces
				#rule %r/[{}]/, Punctuation

				# Identifiers
				rule %r/[A-Za-z_][A-Za-z0-9_]*/, Name

				# Operators
				rule %r{[-+*/&|~!%^<>=]}, Operator

				rule %r/./m, Text
			end
		end
	end
end

