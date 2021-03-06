/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#include "DataItemFormatRepetitive.h"
#include "Tracer.h"
#include "asterixformat.hxx"

DataItemFormatRepetitive::DataItemFormatRepetitive()
{
}

DataItemFormatRepetitive::~DataItemFormatRepetitive()
{
}

long DataItemFormatRepetitive::getLength(const unsigned char* pData)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return 0;
	}
	unsigned char nRepetition = *pData;
	return (1+nRepetition*pFixed->getLength(pData+1));
}

void DataItemFormatRepetitive::addBits(DataItemBits* pBits)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return;
	}
	pFixed->m_lSubItems.push_back(pBits);
}

bool DataItemFormatRepetitive::getText(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength)
{
	bool ret = false;
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return true;
	}

	int fixedLength = pFixed->getLength(pData);
	unsigned char nRepetition = *pData;

	if (1+nRepetition*fixedLength != nLength)
	{
		Tracer::Error("Wrong length in Repetitive");
		return true;
	}

	pData++;

	switch(formatType)
	{
	case CAsterixFormat::EJSON:
	case CAsterixFormat::EJSONH:
	{
		std::string tmpStr = format("[");

		while(nRepetition--)
		{
			ret |= pFixed->getText(tmpStr, strHeader, formatType, pData, fixedLength);
			pData += fixedLength;

			if (nRepetition > 0)
				tmpStr += format(",");
		}
		tmpStr += format("]");

		if (ret)
			strResult += tmpStr;

		break;
	}
	default:
	{
		while(nRepetition--)
		{
			ret |= pFixed->getText(strResult, strHeader, formatType, pData, fixedLength);
			pData += fixedLength;
		}
		break;
	}
	}
	return ret;
}

std::string DataItemFormatRepetitive::printDescriptors(std::string header)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return "Wrong data in Repetitive";
	}

	return pFixed->printDescriptors(header);
}

bool DataItemFormatRepetitive::filterOutItem(const char* name)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return false;
	}

	return pFixed->filterOutItem(name);
}

bool DataItemFormatRepetitive::isFiltered(const char* name)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Repetitive");
		return false;
	}

	return pFixed->isFiltered(name);
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatRepetitive::getWiresharkDefinitions()
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (!pFixed)
	{
		Tracer::Error("Wring format of repetitive item");
		return NULL;
	}
	return pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatRepetitive::getData(unsigned char* pData, long len, int byteoffset)
{
	fulliautomatix_data *lastData = NULL, *firstData = NULL;
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (!pFixed)
	{
		Tracer::Error("Wrong format of repetitive item");
		return NULL;
	}

	int fixedLength = pFixed->getLength(pData);
	unsigned char nRepetition = *pData;

	firstData = lastData = newDataUL(NULL, PID_REP, byteoffset, 1, nRepetition);
	byteoffset+=1;

	if (1+nRepetition*fixedLength != len)
	{
		Tracer::Error("Wrong length in Repetitive");
		return firstData;
	}

	pData++;

	while(nRepetition--)
	{
		lastData->next = pFixed->getData(pData, fixedLength, byteoffset);
		while(lastData->next)
			lastData = lastData->next;

		pData += fixedLength;
		byteoffset += fixedLength;
	}

	return firstData;
}
#endif
