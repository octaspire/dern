# This file is based on documentation from:
# https://github.com/asciidoctor/asciidoctor.org/blob/master/docs/_includes/src-pygments.adoc
require 'pygments'
Pygments.start "#{ENV['HOME']}/hg/pygments-main"
Pygments::Lexer.create name: 'dern', aliases: ['dern'], filenames: ['*.dern'],
    mimetypes: ['text/dern', 'application/x-dern']
