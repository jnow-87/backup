" syntax file
" Language:	backup config (bc)

if exists("b:current_syntax")
  finish
endif

let b:current_syntax = "bc"

" keywords
syntax keyword	bcKeyword			dir config var

" {}-block
syntax region	bcBlock				start="{" end="}" transparent fold

" string
syntax region	bcString			start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=@Spell

" comments
syntax region	bcComment			start="//" skip="\\$" end="$" keepend contains=cTodo,@Spell
syntax region	bcComment			matchgroup=bcComment start="/\*" end="\*/"

" colors
hi def link bcComment				mgreen
hi def link bcKeyword				mblue
hi def link bcString				white

set foldmethod=syntax
