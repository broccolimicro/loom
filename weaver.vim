" Vim syntax file
" Language:	Weaver
" Maintainer:	Ned Bingham
" Latest Revision: 26 April 2025

syn keyword	wFeatures func struct circ proto
syn keyword wInclude import
syn keyword wControlFlow while await and or xor
syn keyword wTypes chan fixed ufixed bool
syn keyword wTodo contained TODO FIXME NOTE DESIGN
syn match   wLineComment "//.*$" contains=wTodo
syn region  wBlockComment start="/\*" end="\*/" contains=wTodo
syn region  wScope transparent start="{" end="}"
syn match   wNumber "[+-]\?\d\+\(\.\d\+\)\?\(e[+-]\?\d\+\)\?"
" syn match   wOperator "[-+*/&|~!%^<>=]"

hi def link wTodo          Todo
hi def link wLineComment   Comment
hi def link wBlockComment  Comment
hi def link wFeatures      Structure
hi def link wControlFlow   Statement
hi def link wTypes         Type
hi def link wInclude       Include
hi def link wNumber        Number
" hi def link wOperator      Operator

let b:current_syntax = "wv"
