<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text"/>
  <xsl:template match="/fix">
    <xsl:text>
#include &lt;map&gt;
#include &lt;quickfix/FieldTypes.h&gt;
#include &lt;string&gt;

#define FIELD_TYPE_STRING_ FIX::TYPE::Type::String
#define FIELD_TYPE_CHAR_ FIX::TYPE::Type::Char
#define FIELD_TYPE_PRICE_ FIX::TYPE::Type::Price
#define FIELD_TYPE_INT_ FIX::TYPE::Type::Int
#define FIELD_TYPE_AMT_ FIX::TYPE::Type::Amt
#define FIELD_TYPE_QTY_ FIX::TYPE::Type::Qty
#define FIELD_TYPE_CURRENCY_ FIX::TYPE::Type::Currency
#define FIELD_TYPE_MULTIPLEVALUESTRING_ FIX::TYPE::Type::MultipleValueString
#define FIELD_TYPE_MULTIPLESTRINGVALUE_ FIX::TYPE::Type::MultipleStringValue
#define FIELD_TYPE_MULTIPLECHARVALUE_ FIX::TYPE::Type::MultipleCharValue
#define FIELD_TYPE_EXCHANGE_ FIX::TYPE::Type::Exchange
#define FIELD_TYPE_UTCTIMESTAMP_ FIX::TYPE::Type::UtcTimeStamp
#define FIELD_TYPE_BOOLEAN_ FIX::TYPE::Type::Boolean
#define FIELD_TYPE_LOCALMKTDATE_ FIX::TYPE::Type::LocalMktDate
#define FIELD_TYPE_DATA_ FIX::TYPE::Type::Data
#define FIELD_TYPE_FLOAT_ FIX::TYPE::Type::Float
#define FIELD_TYPE_PRICEOFFSET_ FIX::TYPE::Type::PriceOffset
#define FIELD_TYPE_MONTHYEAR_ FIX::TYPE::Type::MonthYear
#define FIELD_TYPE_DAYOFMONTH_ FIX::TYPE::Type::DayOfMonth
#define FIELD_TYPE_UTCDATE_ FIX::TYPE::Type::UtcDate
#define FIELD_TYPE_UTCDATEONLY_ FIX::TYPE::Type::UtcDateOnly
#define FIELD_TYPE_UTCTIMEONLY_ FIX::TYPE::Type::UtcTimeOnly
#define FIELD_TYPE_NUMINGROUP_ FIX::TYPE::Type::NumInGroup
#define FIELD_TYPE_PERCENTAGE_ FIX::TYPE::Type::Percentage
#define FIELD_TYPE_SEQNUM_ FIX::TYPE::Type::SeqNum
#define FIELD_TYPE_LENGTH_ FIX::TYPE::Type::Length
#define FIELD_TYPE_COUNTRY_ FIX::TYPE::Type::Country
#define FIELD_TYPE_TIME_ FIX::TYPE::Type::UtcTimeStamp

namespace amphisbaena {
static std::map&lt;std::string, std::tuple&lt;int, FIX::TYPE::Type&gt;, std::less&lt;&gt;&gt;
  g_fields_by_name{</xsl:text>
    <xsl:for-each select="fields/field">
      <xsl:text>
    { "</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>", { </xsl:text>
      <xsl:value-of select="@number"/>
      <xsl:text>, FIELD_TYPE_</xsl:text>
      <xsl:value-of select="@type"/>
      <xsl:text>_ } },</xsl:text>
    </xsl:for-each>
    <xsl:text>
  };

static std::map&lt;int, std::tuple&lt;std::string, FIX::TYPE::Type&gt;&gt;
  g_fields_by_tag{</xsl:text>
    <xsl:for-each select="fields/field">
      <xsl:text>
    { </xsl:text>
      <xsl:value-of select="@number"/>
      <xsl:text>, { "</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>", FIELD_TYPE_</xsl:text>
      <xsl:value-of select="@type"/>
      <xsl:text>_ } },</xsl:text>
    </xsl:for-each>
    <xsl:text>
  };
}</xsl:text>
  </xsl:template>
</xsl:stylesheet>