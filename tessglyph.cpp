/*
    tessglyph.cpp - extract detailed glyph information and/or quick text using Tesseract API

        ./tessglyph -c [config] -e [engine] -i [image] -l [lang] -p [psm] -o [output] -q (quick)
        note: psm is based on Tesseract numbers, see tesseract --help-psm
              engine is based on Tesseract numbers, see tesseract --help-oem
 
    - art rhyno <https://github.com/artunit/>
    (c) Copyright GNU General Public License (GPL)
*/

#include <string>
using std::string;
#include <iostream>
#include <sstream>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#define UTF8_ENCODING "UTF-8"

tesseract::PageSegMode sortOutPsms(int psm)
{
    if (psm == 0) return tesseract::PSM_AUTO;
    if (psm == 1) return tesseract::PSM_AUTO_ONLY;
    if (psm == 2) return tesseract::PSM_AUTO_OSD;
    if (psm == 3) return tesseract::PSM_CIRCLE_WORD;
    if (psm == 4) return tesseract::PSM_AUTO_OSD;
    if (psm == 5) return tesseract::PSM_RAW_LINE;
    if (psm == 6) return tesseract::PSM_SINGLE_BLOCK;
    if (psm == 7) return tesseract::PSM_SINGLE_BLOCK_VERT_TEXT;
    if (psm == 8) return tesseract::PSM_SINGLE_CHAR;
    if (psm == 9) return tesseract::PSM_SINGLE_COLUMN;
    if (psm == 10) return tesseract::PSM_SINGLE_LINE;
    if (psm == 11) return tesseract::PSM_SINGLE_WORD;
    if (psm == 12) return tesseract::PSM_SPARSE_TEXT;
    if (psm == 13) return tesseract::PSM_SPARSE_TEXT_OSD;
    return tesseract::PSM_AUTO_OSD;
}//sortOutPsms

tesseract::OcrEngineMode sortOutEngines(int engine)
{
    if (engine == 0) return tesseract::OEM_TESSERACT_ONLY;
    if (engine == 1) return tesseract::OEM_LSTM_ONLY;
    if (engine == 2) return tesseract::OEM_TESSERACT_LSTM_COMBINED;
    if (engine == 3) return tesseract::OEM_DEFAULT;
    if (engine == 4) return tesseract::OEM_CUBE_ONLY;
    if (engine == 5) return tesseract::OEM_TESSERACT_CUBE_COMBINED;
    return tesseract::OEM_DEFAULT;
}//sortOutEngines


//all this is destined for ALTO goodness
void showFontInfo(const char* font_name, bool is_bold, bool is_italic, 
    bool is_underlined, bool is_monospace, bool is_serif, bool is_smallcaps,
    int pointsize, int font_id) 
{
    printf("     Font details:\n");
    printf("      name: %s\n", font_name);
    printf("      bold: %s\n",(is_bold ? "true" : "false"));
    printf("      italic: %s\n",(is_italic ? "true" : "false"));
    printf("      underlined: %s\n",(is_underlined ? "true" : "false"));
    printf("      monospace: %s\n",(is_monospace ? "true" : "false"));
    printf("      serif: %s\n",(is_serif ? "true" : "false"));
    printf("      smallcaps: %s\n",(is_smallcaps ? "true" : "false"));
    printf("      pointsize: %d\n", pointsize);
    printf("      font_id: %d\n", font_id);
}//showFontInfo

//write ALTO format 
void writeXmltoFile(const char *alto_file, tesseract::TessBaseAPI *api)
{
    int rc;
    xmlTextWriterPtr writer;
    xmlChar *tmp;

    bool bold, italic, underlined, monospace, serif, smallcaps; 
    int pointsize, font_id;

    // Get OCR result
    tesseract::ResultIterator* ri = api->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;

    if(ri != 0) {
        /* Create a new XmlWriter for file, with no compression. */
        writer = xmlNewTextWriterFilename(alto_file, 0);
        if (writer == NULL) {
            printf("writeXmltoFile: Error creating the xml writer\n");
            return;
        }
        xmlTextWriterSetIndent(writer,1);
        xmlTextWriterStartDocument(writer, NULL, UTF8_ENCODING, NULL);

        xmlTextWriterWriteFormatComment(writer,"%s\n%s",
            "Simple layout based on ALTO, see: https://www.loc.gov/standards/alto/",
            "see also Glyph discussion: https://github.com/altoxml/schema/issues/26");
 

        //Glyph symbol syntax not decided yet in ALTO so don't use
        //namespace yet, see: https://github.com/altoxml/schema/issues/26
        /* when decided on, use this:
        xmlTextWriterStartElementNS(writer, BAD_CAST "alto", BAD_CAST "TextBlock", 
            BAD_CAST "http://www.loc.gov/standards/alto/ns-v2#");
        */
        xmlTextWriterStartElement(writer, BAD_CAST "TextBlock");
        xmlTextWriterWriteAttribute(writer, BAD_CAST "ID", BAD_CAST "A1");

        //bool block_started = false;
        //bool line_started = false;
        bool word_started = false;
        do {
            const char* symbol = ri->GetUTF8Text(level);
            //skip PARA & TEXTLINE for now - sometimes sequence is off  
            // - also, probably better to use geometry approach?
            /*
            if (ri->IsAtBeginningOf(tesseract::RIL_PARA)) { 
                if (block_started) xmlTextWriterEndElement(writer);
                xmlTextWriterStartElement(writer, BAD_CAST "TextBlock");
                block_started = true;
            }
            if (ri->IsAtBeginningOf(tesseract::RIL_TEXTLINE)) { 
                if (line_started) xmlTextWriterEndElement(writer);
                xmlTextWriterStartElement(writer, BAD_CAST "TextLine");
                line_started = true;
            }
            */
            if (symbol != 0 && ri->IsAtBeginningOf(tesseract::RIL_WORD)) {
                if (word_started) xmlTextWriterEndElement(writer);
                xmlTextWriterStartElement(writer, BAD_CAST "String");
                word_started = true;
                //TODO: push through font informatiion in ALTO
                /*
                printf("---->Start word\n");
                const char *font_name = ri->WordFontAttributes(&bold,   
                   &italic, &underlined,&monospace, &serif, &smallcaps,&pointsize, &font_id);
                showFontInfo(font_name,bold,italic,underlined,monospace,serif,smallcaps,
                    pointsize,font_id);
                */
            }
            if (symbol != 0) {
                int x1, y1, x2, y2;
                ri->BoundingBox(level, &x1, &y1, &x2, &y2);
                tesseract::ChoiceIterator ci(*ri);
                bool start = true;
                do {
                    const char* choice = ci.GetUTF8Text();
                    if (start) {
                        xmlTextWriterStartElement(writer, BAD_CAST "Glyph");
                        xmlTextWriterWriteFormatAttribute(writer,BAD_CAST "HPOS","%d",x1);
                        xmlTextWriterWriteFormatAttribute(writer,BAD_CAST "VPOS","%d",y1);
                        xmlTextWriterWriteFormatAttribute(writer,BAD_CAST "WIDTH","%d",(x2 - x1));
                        xmlTextWriterWriteFormatAttribute(writer,BAD_CAST "HEIGHT","%d",(y2 - y1));
                        start = false;
                    } 
                    xmlTextWriterStartElement(writer, BAD_CAST "Variant");
                    xmlTextWriterWriteFormatAttribute(writer,BAD_CAST "VC","%2.6lf",ci.Confidence());
                    xmlTextWriterWriteFormatString(writer,"%s",choice);
                    xmlTextWriterEndElement(writer);
                } while(ci.Next());
                if (!start) xmlTextWriterEndElement(writer);
            }//if
            delete[] symbol;

        } while((ri->Next(level)));
                
        if (word_started) xmlTextWriterEndElement(writer);
        //if (line_started) xmlTextWriterEndElement(writer);
        //if (block_started)xmlTextWriterEndElement(writer);
        xmlTextWriterEndElement(writer); //TextBlock end
    }//if

    xmlFreeTextWriter(writer);
}

int main(int argc, char* argv[])
{
    const char* default_lang = "eng";
    const char* default_img = "default.jpg";
    const char* default_alto = "alto.xml";
    int default_psm = 4;
    bool quick_flag = false;
    int default_engine = 3;
    const char *config = "config"; 
 
    std::string lang = default_lang;
    std::string img = default_img;
    std::string alto_file = default_alto;
    int psm = default_psm;
    int engine = default_engine;

    //sort out parameters
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-e") {
            if (i + 1 < argc) { 
                engine = atoi(argv[++i]);
            } else { 
                std::cerr << "-p option requires engine argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-c") {
            if (i + 1 < argc) { 
                config = argv[++i]; 
            } else { 
                std::cerr << "-c option requires config argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-i") {
            if (i + 1 < argc) { 
                img = argv[++i]; 
            } else { 
                std::cerr << "-i option requires image argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-l") {
            if (i + 1 < argc) { 
                lang = argv[++i]; 
            } else { 
                std::cerr << "-l option requires lang argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-p") {
            if (i + 1 < argc) { 
                psm = atoi(argv[++i]);
            } else { 
                std::cerr << "-p option requires psm argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-o") {
            if (i + 1 < argc) { 
                alto_file = argv[++i];
            } else { 
                std::cerr << "-o option requires alto file argument." << std::endl;
                return 0;
            }//if  
        }//if 
        if (std::string(argv[i]) == "-q") quick_flag = true;
    }//for

    //set up API
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    api->SetPageSegMode(sortOutPsms(psm));
    
    char *configs[]={(char *)config};
    int configs_size = 1;

    if (api->Init(NULL, lang.c_str(), sortOutEngines(engine), 
        configs, configs_size, NULL, NULL, false)) 
    {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }//if

    Pix *image = pixRead(img.c_str());
    api->SetImage(image);
    //Important
    api->Recognize(NULL);

    if (quick_flag) {
        char *quick_text = api->GetUTF8Text();
        fprintf(stdout,"%s",quick_text);
    } else {
        writeXmltoFile(alto_file.c_str(), api);
    }//if

    // Destroy used object and release memory
    api->End();
    pixDestroy(&image);
                

    return 0;
}
