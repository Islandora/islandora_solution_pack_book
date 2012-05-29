<?xml version="1.0" encoding="UTF-8"?>
<!--takes incoming hocr and outputs it as is except for text which it changes to lowercase-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0"
    xmlns:hocr="http://www.w3.org/1999/xhtml" xmlns="http://www.w3.org/1999/xhtml">
    <xsl:output method="xml"
        encoding="UTF-8"
        indent="yes"/>
    <xsl:variable name="smallcase" select="'abcdefghijklmnopqrstuvwxyz'" />
    <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ.,?!:;'" />
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
        </xsl:template>
    <xsl:template match="hocr:span[@class='ocrx_word']"><!-- this template for hocr created by tranfroming abbyy wordsxy-->
        <xsl:variable name = "WordID"><xsl:value-of select="@id"></xsl:value-of></xsl:variable>
        <xsl:element   name = "span">
            <xsl:attribute name="id">
                <xsl:value-of select="$WordID"/>
            </xsl:attribute>
            <xsl:attribute name="title">x_wconf 0</xsl:attribute>
            <xsl:attribute name="class">ocrx_word</xsl:attribute>
            <xsl:value-of select="translate(.,$uppercase,$smallcase)"/>
        </xsl:element>
        
    </xsl:template>
    <xsl:template match="span[@class='ocrx_word']"><!--this template for hocr created with  tesseract-->
        <xsl:variable name = "WordID"><xsl:value-of select="@id"></xsl:value-of></xsl:variable>
        <xsl:variable name = "Confidence"><xsl:value-of select="@title"></xsl:value-of></xsl:variable>
        <xsl:element   name = "span">
            <xsl:attribute name="id">
                <xsl:value-of select="$WordID"/>
            </xsl:attribute>
            <xsl:attribute name="title"><xsl:value-of select="$Confidence"/></xsl:attribute>
            <xsl:attribute name="class">ocrx_word</xsl:attribute>
            <xsl:value-of select="translate(.,$uppercase,$smallcase)"/>
        </xsl:element>
        
    </xsl:template>
</xsl:stylesheet>