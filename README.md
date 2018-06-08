# tessglyph

The [Tesseract OCR system](https://github.com/tesseract-ocr) has a powerful 
API that opens up access to the internals of its OCR processing, including the 
variants considered for individual characters/gylphs. This program provides the 
building blocks for including detailed variant and font information in the 
[ALTO XML format](https://www.loc.gov/standards/alto/). Our workflow typically involves 
the use of the 
[Olena project](http://www.lrde.epita.fr/cgi-bin/twiki/view/Olena/WebHome) to 
segment pages into the 
[PAGE format](http://schema.primaresearch.org/tools/PAGELibraries), and then 
to process individual paragraphs with Tesseract. This means that Tesseract 
typically does not see the entire page at once so we overload the 
_String_ element with font details, and then sort out the
ultimate ALTO representation after the page has been processed. For example:

```xml
 <String CONTENT="This" WC="96.144791" BOLD="0" ITALIC="0" UNDERLINED="0" MONOSPACE="0" SERIF="0" SMALLCAPS="1" POINTSIZE="11" FONT="Arial">
  <Glyph CONTENT="T" HPOS="36" VPOS="92" WIDTH="14" HEIGHT="24" GC="99.558067"/>
   <Variant VC="69.500244">+</Variant>
  </Glyph>
  <Glyph CONTENT="h" HPOS="50" VPOS="92" WIDTH="5" HEIGHT="24" GC="99.546646"/>
  <Glyph CONTENT="i" HPOS="62" VPOS="92" WIDTH="9" HEIGHT="24" GC="99.449257"/>
  <Glyph CONTENT="s" HPOS="76" VPOS="92" WIDTH="19" HEIGHT="24" GC="99.574234"/>
 </String>
```

At the other end of the spectrum, we also have need for the simple text from 
Tesseract, and often find it convenient to be able to produce both at the same 
time. The options provided are:

```
-c
    Tesseract config file (if desired)
-e
    Tesseract engine number, see tesseract --help-oem
-i
    input image (defaults to "default.jpg")
-l
    language code for OCR (defaults to "eng")
-p
    Tesseract psm number, see tesseract --help-psm 
-o
    output file (defaults to "alto.xml")
-q
    quick flag, for producing text only on stdout
-b
    both flag, for producing both text on stdout and XML file at the same time
```

We often make use of Tesseract's 
[LSTM option](https://github.com/tesseract-ocr/tesseract/wiki/4.0-with-LSTM)
though it does not currently provide font identification through the API. We
did use a workaround for this, which would invoke both the
LSTM engine and the legacy engine, but this is expensive computationally
and there are other approaches to getting font information, so we no longer
provide this option. Note that using an _engine number_ of 2 will sometimes 
produce font details. For more information, see
this [Tesseract issue](https://github.com/tesseract-ocr/tesseract/issues/1074).

A typical invocation would be:

```
./tessglyph -e 2 -o phototest.xml -p 6 -i phototest.tif -b > phototest.txt
```

Again, this is in the context of using external segmentation and producing 
an ALTO file further downstream.
 
