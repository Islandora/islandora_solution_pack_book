<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:php="http://php.net/xsl">
  <xsl:param name="OBJECTSPAGE"/>
  <xsl:param name="PID"/>
  <xsl:param name="USER"/>
  <xsl:param name="INGESTED"/>
  <xsl:param name="SOLRFIELD"/>

  <xsl:template match="/">

    <div id="islandora-book-view">
      <table class="islandora-book-table">
        <tr><td>
          <img>
            <xsl:attribute name="src"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:copy-of select="$PID"/>/TN
            </xsl:attribute>
          </img>
        </td>
        <td>
          <div style="align:left">
            <table class="islandora-book-table" cellpadding="3" cellspacing="2" width="90%">
              <tr>
                <xsl:if test="//mods:note[@type='statement of responsibility']">
                  <td align="right" valign="top"><b>By Statement: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:note[@type='statement of responsibility']"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:placeTerm[@type='text']">
                  <td align="right" valign="top"><b>Place of Publication: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:placeTerm[@type='text']"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:publisher">
                  <td align="right" valign="top"><b>Publisher: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:publisher"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:dateIssued">
                  <td align="right" valign="top"><b>Date: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:dateIssued"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:languageTerm">
                  <td align="right" valign="top"><b>Language: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:languageTerm"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:extent">
                  <td align="right" valign="top"><b>Pagination: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:extent"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:identifier[@type='isbn']">
                  <td align="right" valign="top"><b>ISBN 10: </b></td>
                  <td valign="top"><xsl:value-of select="//mods:identifier[@type='isbn']"/></td>
                </xsl:if>
              </tr>
              <tr>
                <xsl:if test="//mods:subject">
                  <td align="right" valign="top"><b>Subjects: </b></td>
                  <td valign="top">
                    <xsl:for-each select="//mods:subject">
                      <xsl:for-each select="*">
                        <a><xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>islandora/solr/search/<xsl:value-of select="$SOLRFIELD"/>:"<xsl:value-of select="normalize-space(.)"/>"</xsl:attribute><xsl:value-of select="."/></a>
                        <xsl:text> </xsl:text>
                      </xsl:for-each>
                      <br />
                    </xsl:for-each>
                  </td>
                </xsl:if>
              </tr>
            </table>
          </div>
        </td>
       
        <td valign="top">
          <xsl:if test="$INGESTED = 'true'">
            <div><a><xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/book_viewer/<xsl:value-of select="$PID"/></xsl:attribute>Read</a></div>
          </xsl:if>
          <xsl:if test="$INGESTED = 'true'">
            <div><a><xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$PID"/>/PDF/<xsl:value-of select="$PID"/>.pdf</xsl:attribute>Download</a></div>
          </xsl:if>
          <xsl:if test="($USER > 0) and ($INGESTED = 'true')">
            <div><a><xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>teieditor/<xsl:value-of select="$PID"/></xsl:attribute>Edit</a></div>
          </xsl:if>
        </td></tr>
      </table>
    </div>
  </xsl:template>
</xsl:stylesheet>
