<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  extension-element-prefixes="str exsl">

  <!-- 
       Print the XML files in CoNLL-X format. Currently only: form,
       lemma, coarse-grained POS, and POS. The features field contains
       'active:1' for any lexical nodes that match the query.
  -->

  <xsl:output method="text" encoding="UTF-8"/>

  <xsl:param name='filename'/>

  <xsl:variable name="selectedNodes" select="//node[@active='1']"/>
  <xsl:variable name="words" select="str:tokenize(/*/sentence)"/>

  <xsl:template match="/">
    <xsl:for-each select="//node[@root]">
      <xsl:sort select="@begin" data-type="number"/>
      <xsl:value-of select="@end" />
      <xsl:text>&#x9;</xsl:text>
      <xsl:value-of select="@word" />
      <xsl:text>&#x9;</xsl:text>
      <xsl:value-of select="@lemma" />
      <xsl:text>&#x9;</xsl:text>
      <xsl:value-of select="@pt" />
      <xsl:text>&#x9;</xsl:text>
      <xsl:value-of select="@postag" />
      <xsl:text>&#x9;</xsl:text>
      <xsl:choose>
        <xsl:when test="@active = '1'">
          <xsl:text>active:1</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>_</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:text>&#xA;</xsl:text>
    </xsl:for-each>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
