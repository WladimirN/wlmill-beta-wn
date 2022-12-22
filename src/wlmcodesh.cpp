#include "wlmcodesh.h"

WLMCodeSH::WLMCodeSH(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	 HighlightingRule rule;
	
     classFormat.setFontWeight(QFont::Bold);
     classFormat.setForeground(Qt::darkMagenta);
     rule.pattern = QRegExp("function.+[)]");
     rule.format = classFormat;
     highlightingRules.append(rule);

	 classFormat.setFontWeight(QFont::Bold);
     classFormat.setForeground(Qt::magenta);
	 rule.pattern = QRegExp("return");
     rule.format = classFormat;
     highlightingRules.append(rule);

	 singleLineCommentFormat.setForeground(Qt::gray);
     rule.pattern = QRegExp("//[^\n]*");
     rule.format = singleLineCommentFormat;
     highlightingRules.append(rule);

     commentStartExpression = QRegExp("/\\*");
     commentEndExpression = QRegExp("\\*/");
	
}

WLMCodeSH::~WLMCodeSH()
{

}

void WLMCodeSH::addKeywords(QStringList keywordPatterns, QColor color)
{
HighlightingRule rule;

keywordFormat.setForeground(color);
keywordFormat.setFontWeight(QFont::Bold);
/*
QStringList keywordPatterns;
keywordPatterns << "\\bDIALOG\\b" << "\\bAXIS\\b" << "\\bTIMER\\b"
                << "\\bDIN\\b" << "\\bDOUT\\b" << "\\bK\\b"
                << "\\bFILE\\b" << "\\bMACHINE\\b"<< "\\bGCODE\\b"
                << "\\bGPROGRAM\\b"<< "\\bSCRIPT\\b"<< "\\bVALUES\\b";
*/
foreach (const QString &pattern, keywordPatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.prepend(rule);
}
}

 void WLMCodeSH::highlightBlock(const QString &text)
 {
     foreach (const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
     setCurrentBlockState(0);

     int startIndex = 0;
     if (previousBlockState() != 1)
         startIndex = commentStartExpression.indexIn(text);

     while (startIndex >= 0) {
         int endIndex = commentEndExpression.indexIn(text, startIndex);
         int commentLength;
         if (endIndex == -1) {
             setCurrentBlockState(1);
             commentLength = text.length() - startIndex;
         } else {
             commentLength = endIndex - startIndex
                             + commentEndExpression.matchedLength();
         }
         //setFormat(startIndex, commentLength, multiLineCommentFormat);
         setFormat(startIndex, commentLength, singleLineCommentFormat);
         startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
     }
 }
