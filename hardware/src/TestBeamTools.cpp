#include "TestBeamTools.h"

#include "TAlpide.h"
#include <iostream>
#include <tinyxml.h>

unsigned int Bitmask(int width)
{
  unsigned int tmp = 0;
  for (int i = 0; i < width; i++)
    tmp |= 1 << i;
  return tmp;
}

void ParseXML(TAlpide *dut, TiXmlNode *node, int base, int rgn, bool readwrite)
{
  // readwrite (from Chip): true = read; false = write
  for (TiXmlNode *pChild = node->FirstChild("address"); pChild != 0;
       pChild            = pChild->NextSibling("address")) {
    if (pChild->Type() != TiXmlNode::TINYXML_ELEMENT) continue;
    //         printf( "Element %d [%s] %d %d\n", pChild->Type(), pChild->Value(),
    //         base, rgn);
    TiXmlElement *elem = pChild->ToElement();
    if (base == -1) {
      if (!elem->Attribute("base")) {
        std::cout << "Base attribute not found!" << std::endl;
        break;
      }
      ParseXML(dut, pChild, atoi(elem->Attribute("base")), -1, readwrite);
    }
    else if (rgn == -1) {
      if (!elem->Attribute("rgn")) {
        std::cout << "Rgn attribute not found!" << std::endl;
        break;
      }
      ParseXML(dut, pChild, base, atoi(elem->Attribute("rgn")), readwrite);
    }
    else {
      if (!elem->Attribute("sub")) {
        std::cout << "Sub attribute not found!" << std::endl;
        break;
      }
      int      sub     = atoi(elem->Attribute("sub"));
      uint16_t address = ((rgn << 11) + (base << 8) + sub);
      uint16_t value   = 0;
      // std::cout << "region" << rgn << " " << base << " " << sub << std::endl;

      if (readwrite) {
        if (dut->ReadRegister(address, value) != 1) {
          std::cout << "Failure to read chip address " << address << std::endl;
          continue;
        }
      }

      for (TiXmlNode *valueChild = pChild->FirstChild("value"); valueChild != 0;
           valueChild            = valueChild->NextSibling("value")) {
        if (!valueChild->ToElement()->Attribute("begin")) {
          std::cout << "begin attribute not found!" << std::endl;
          break;
        }
        int begin = atoi(valueChild->ToElement()->Attribute("begin"));

        int width = 1;
        if (valueChild->ToElement()->Attribute("width")) // width attribute is
                                                         // optional
          width = atoi(valueChild->ToElement()->Attribute("width"));

        if (!valueChild || !valueChild->FirstChild("content") ||
            !valueChild->FirstChild("content")->FirstChild()) {
          std::cout << "Content tag not found! Base: " << base << ", region: " << rgn
                    << ", sub: " << sub << std::endl;
          printf("Element %d [%s] %d %d\n", pChild->Type(), pChild->Value(), base, rgn);
          break;
        }
        if (readwrite) {
          int  subvalue = (value >> begin) & Bitmask(width);
          char tmp[9];
          sprintf(tmp, "0x%X", subvalue);
          valueChild->FirstChild("content")->FirstChild()->SetValue(tmp);
        }
        else {
          int content =
              (int)strtol(valueChild->FirstChild("content")->FirstChild()->Value(), 0, 16);

          if (content >= (1 << width)) {
            std::cout << "value too large: " << begin << " " << width << " " << content << " "
                      << valueChild->FirstChild("content")->Value() << std::endl;
            std::cout << "Base: " << base << ", region: " << rgn << ", sub: " << sub << std::endl;
            break;
          }
          value += content << begin;
        }
      }
      if (!readwrite) {
        // printf("%d %d %d: %d %d\n", base, rgn, sub, address, value);
        if (dut->WriteRegister(address, value) != 1)
          std::cout << "Failure to write chip address " << address << std::endl;
        uint16_t valuecompare = -1;
        if (dut->ReadRegister(address, valuecompare) != 1)
          std::cout << "Failure to read chip address after writing chip address " << address
                    << std::endl;
        if (address != 14 && value != valuecompare)
          std::cout << "Register 0x" << std::hex << address << std::dec
                    << "read back error : write value is : " << value
                    << " and read value is : " << valuecompare << std::endl;
      }
    }
  }
}
