-- GitHub hides the comment; Pandoc turns it into an intentional PDF page break.
function RawBlock(el)
  if el.format == 'html' and el.text:match('^<!%-%-%s*pagebreak%s*%-%->$') then
    if FORMAT:match('latex') then
      return pandoc.RawBlock('latex', '\\clearpage')
    end
    return {}
  end
end
