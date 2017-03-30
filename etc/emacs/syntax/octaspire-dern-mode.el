; Based on tutorial wron emacswiki.org/emacs/ModeTutorial

(defvar octaspire-dern-mode-hook nil)

(defvar octaspire-dern-mode-map
  (let ((map (make-keymap)))
    (define-key map "\C-j" 'newline-and-indent)
    map)
  "Keymap for octaspire-dern major mode")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.dern\\'" . octaspire-dern-mode))

;(regexp-opt '("println" "env-new" "env-current" "env-global" "\+\+" "--" "\*" "-" "find" "hash-map" "exit" "mutable" "doc" "len" "read-and-eval-path" "read-and-eval-string" "define" "quote" "if" "while" "for" "=" "==" "\<\=" "fn") t)"\\(\\+\\+\\|--\\|<=\\|==\\|d\\(?:efine\\|oc\\)\\|e\\(?:nv-\\(?:current\\|global\\|new\\)\\|xit\\)\\|f\\(?:ind\\|n\\|or\\)\\|hash-map\\|if\\|len\\|mutable\\|println\\|quote\\|read-and-eval-\\(?:path\\|string\\)\\|while\\|[*=-]\\)" 

;(regexp-opt '("nil" "false" "true") t)

;(defconst octaspire-dern-lock-keywords-1
;  (list
;   '("\\<\\(\\+\\+\\|--\\|<=\\|==\\|d\\(?:efine\\|oc\\)\\|e\\(?:nv-\\(?:current\\|global\\|new\\)\\|xit\\)\\|f\\(?:alse\\|ind\\|n\\|or\\)\\|hash-map\\|if\\|len\\|mutable\\|nil\\|println\\|quote\\|read-and-eval-\\(?:path\\|string\\)\\|\\(?:tru\\|whil\\)e\\|[*=-]\\)\\>" . font-lock-builtin-face)
;   '("\\(false\\|nil\\|true\\)" . font-lock-constant-face)
;   '("\\('\\w*'\\)" . font-lock-variable-name-face))
;  "Minimal highlighting expressions for octaspire-dern mode")

(defconst octaspire-dern-lock-keywords-1
  (list
   (cons (regexp-opt '("println" "env-new" "env-current" "env-global" "\+" "\+\+" "--" "\*" "-" "find" "hash-map" "exit" "mutable" "do" "doc" "len" "read-and-eval-path" "read-and-eval-string" "define" "nth" "quote" "if" "while" "for" "\=" "\=\=" ">" ">\=" "<" "<\=" "fn") t) font-lock-builtin-face)
   (cons (regexp-opt '("nil" "false" "true") t) font-lock-constant-face)
   (cons "\\('\\w*'\\)" font-lock-variable-name-face)
   (cons "\\('\\d*'\\)" font-lock-constant-face)
   )
   "Minimal highlighting expressions for octaspire-dern mode")


(defvar octaspire-dern-font-lock-keywords octaspire-dern-lock-keywords-1
  "Default highlighting expressions for octaspire-dern mode")

;TODO XXX these rules thing, for example, ]cat] as a string.
(defvar octaspire-dern-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?\; "<"   st)
    (modify-syntax-entry ?\n ">"   st)
    (modify-syntax-entry ?'  "'"   st)
    (modify-syntax-entry ?(  "()"  st)
    (modify-syntax-entry ?)  ")("  st)
    (modify-syntax-entry ?[  "|"   st)
    (modify-syntax-entry ?]  "|"   st)
    ;(modify-syntax-entry ?\|  "/"   st)
    (modify-syntax-entry ?|  "/"   st)
    st)
  "Syntax table for octaspire-dern-mode")

(defun octaspire-dern-mode ()
 "Major mode for editing octaspire dern language files"
 (interactive)
 (kill-all-local-variables)
 (set-syntax-table octaspire-dern-mode-syntax-table)
 (use-local-map octaspire-dern-mode-map)
 (set (make-local-variable 'font-lock-defaults) '(octaspire-dern-font-lock-keywords))
 (setq major-mode 'octaspire-dern-mode)
 (setq mode-name "OCTASPIRE DERN")
 (run-hooks 'octaspire-dern-mode-hook))

(show-paren-mode 1)

(provide 'octaspire-dern-mode)
