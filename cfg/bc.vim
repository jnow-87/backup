" syntax file
" Language:	backup config (bc)

if exists("b:current_syntax")
  finish
endif

let b:current_syntax = "bc"

" keywords
syntax keyword	bcType			dir config var

syntax match	bcMember		"\.\(archive\|out-dir\|tmp-dir\|log-file\|rsync-dir\|verbose\|indicate\|preserve\|file\|name\|noconfig\|nodata\)"

" {}-block
syntax region	bcBlock				start="{" end="}" transparent fold

" string
syntax region	bcString			start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=@Spell

" comments
syntax region	bcComment			start="//" skip="\\$" end="$" keepend contains=cTodo,@Spell
syntax region	bcComment			matchgroup=bcComment start="/\*" end="\*/"

" colors
hi def link bcComment				mgreen
hi def link bcType					mblue
hi def link bcMember				mlblue
hi def link bcString				white

setlocal foldmethod=syntax
setlocal commentstring=/*%s*/
