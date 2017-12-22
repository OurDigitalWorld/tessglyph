# tessglyph

The [Tesseract OCR system](https://github.com/tesseract-ocr) has a powerful 
API that opens up access to the internals of its OCR processing, including the 
variants considered for individual characters/gylphs. This program provides the building blocks for including detailed
variant and font information in the 
[ALTO XML format](https://www.loc.gov/standards/alto/). Our workflow involves 
the use of the 
[Olena project](http://www.lrde.epita.fr/cgi-bin/twiki/view/Olena/WebHome) to 
segment pages into the 
[PAGE format](http://schema.primaresearch.org/tools/PAGELibraries), and then 
to process individual paragraphs with Tesseract. This means that Tesseract 
typically does not see the entire page at once and we overload the 
_String_ element with font details, and then sort out the
ultimate _ALTO_ representation after the page has been processed. For example:

```xml
 <String CONTENT="This" WC="96.144791" BOLD="0" ITALIC="0" UNDERLINED="0" MONOSP
ACE="0" SERIF="0" SMALLCAPS="53" POINTSIZE="11" FONT="Arial">
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
    Tesseract engine number, see _tesseract --help-oem_
-i
    input image (defaults to "default.jpg")
-l
    language code for OCR (defaults to "eng")
-p
    Tesseract psm number, see _tesseract --help-psm_ 
-o
    output file (defaults to "alto.xml")
-q
    _quick_ flag, for producing text only on stdout
-b
    _both_ flag, for producing both text on stdout and XML file at the same time
```

We often make use of Tesseract's 
[LSTM option](https://github.com/tesseract-ocr/tesseract/wiki/4.0-with-LSTM)
but it does not currently provide font identification through the API. To
work around this, we define an _engine number_ of 100 to invoke both the
LSTM engine and the legacy engine. In this scenario, the legacy engine 
provides only font identification. This is an expensive workaround in terms 
of computation but is currently the only option for extracting fonts 
in combination with _LSTM_. For more information, see
this [Tesseract issue](https://github.com/tesseract-ocr/tesseract/issues/1074).

A typical invocation for us would be:

```
./tessglyph -e 100 -o phototest.xml -p 6 -i phototest.tif -b > phototest.txt
```

Again, this is in the context of using external segmentation and producing 
an _ALTO_ file further downstream.
 
