<?xml version="1.0"?>
<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:template match="ai:DeviceDesc" xmlns:ai="http://www.auto.tuwien.ac.at/~mkoegler/eib/xml/appinfo.xsd">
#
# Device Description
#
msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"MIME-Version: 1.0\n"

<xsl:for-each select="ai:Description"><xsl:for-each select="ai:Title">
#Description/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#Description/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:Copyright">
#Description/Copyright
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:Author">
#Description/Author
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:Manufacturer">
#Description/Manufacturer
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:Category">
#Description/Category
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:OrderNo">
#Description/OrderNo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>
<xsl:for-each select="ai:FunctionalBlock"><xsl:for-each select="ai:Title">
#FunctionalBlock/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#FunctionalBlock/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:Interface"><xsl:for-each select="ai:Title">
#Interface/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#Interface/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each></xsl:for-each>

<xsl:for-each select="ai:GroupObject"><xsl:for-each select="ai:Title">
#GroupObject/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#GroupObject/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>

<xsl:for-each select="ai:Property"><xsl:for-each select="ai:Title">
#Property/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#Property/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>

<xsl:for-each select="ai:PollingMaster"><xsl:for-each select="ai:Title">
#PollingMaster/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#PollingMaster/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>
<xsl:for-each select="ai:PollingSlave"><xsl:for-each select="ai:Title">
#PollingSlave/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="ai:AddInfo">
#PollingSlave/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>
<xsl:for-each select="ai:Parameter"><xsl:for-each select="*/ai:Title">
#Parameter/Title
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="*/ai:AddInfo">
#Parameter/AddInfo
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="*/ai:Unit">
#Parameter/Unit
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each><xsl:for-each select="*/ai:ListElement">
#Parameter/ListElement
msgid "<xsl:value-of select="."/>"
msgstr ""

</xsl:for-each></xsl:for-each>

</xsl:template>
</xsl:transform>
