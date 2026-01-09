/****************************************************************************
* FusionAnalyser - Digital Dental Model Analysis Software
*
* Copyright (C) 2024-2026 AI-Align (基骨智能)
*
* This file is part of FusionAnalyser.
*
* FusionAnalyser is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* FusionAnalyser is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with FusionAnalyser. If not, see <https://www.gnu.org/licenses/>.
*
* SPDX-License-Identifier: GPL-3.0-or-later
****************************************************************************/

#ifndef ML_EXCEPTION_H
#define ML_EXCEPTION_H

#include <exception>

class MLException : public std::exception
{
public:
	MLException(const QString& text)
		:std::exception(),excText(text){_ba = excText.toLocal8Bit();}

	~MLException() throw() {}
	inline const char* what() const throw() {return _ba.constData();}

protected:
	QString excText;
	QByteArray _ba;

};

class MeshLabXMLParsingException : public MLException
{
public:
        MeshLabXMLParsingException(const QString& text)
                :MLException(QString("Error While parsing the XML filter plugin descriptors: ") + text){}

        ~MeshLabXMLParsingException() throw() {}
};

class ParsingException : public MLException
{
public:
        ParsingException(const QString& text)
                :MLException(QString("Parsing Error: ") + text){}

        ~ParsingException() throw() {}
};

class ValueNotFoundException : public MLException
{
public:
        ValueNotFoundException(const QString& valName)
                :MLException(QString("Value Name: ") + valName +  QString(" has not been defined in current environment.")){}

        ~ValueNotFoundException() throw() {}
};

class NotConstException : public MLException
{
public:
	NotConstException(const QString& exp)
		:MLException(QString("Expression: ") + exp +  QString(" is not a const expression. Expression contains an assignment operator \"=\".")){}

	~NotConstException() throw() {}
};

class QueryException : public MLException
{
public:
	QueryException(const QString& syntaxError)
		:MLException(QString("Query Error: ") + syntaxError){}

	~QueryException() throw() {}
};

class JavaScriptException : public MLException
{
public:
	JavaScriptException(const QString& syntaxError)
		:MLException(QString("JavaScript Error: ") + syntaxError){}

	~JavaScriptException() throw() {}
};

class ExpressionHasNotThisTypeException :  public MLException
{
public:
	ExpressionHasNotThisTypeException(const QString& expectedType,const QString& exp)
		:MLException(QString("Expression: ") + exp + " cannot be evaluated to a " + expectedType + "'s value."){}

	~ExpressionHasNotThisTypeException() throw() {}
};

class InvalidInvariantException : public MLException
{
public:
	InvalidInvariantException(const QString& invarianterror)
		:MLException(QString("WARNING! Invalid Invariant: ") + invarianterror){}

	~InvalidInvariantException() throw() {}
};
#endif
