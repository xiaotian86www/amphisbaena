#!/bin/python

import lxml.etree as ET

meta = ET.parse("../thirdparty/quickfix/spec/FIX42.xml")
xslt = ET.parse("../src/plugin/fix_client/fix_field.hpp.xsl")
transform = ET.XSLT(xslt)
output = transform(meta)

with open("../src/plugin/fix_client/fix_field.hpp", "w", encoding="utf-8") as f:
    f.write(str(output))