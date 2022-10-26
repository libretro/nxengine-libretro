
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nx.h"
#include "../common/DBuffer.h"
#include "../common/DString.h"
#include "../common/bufio.h"
#include "../common/StringList.h"

#include "sectStringArray.h"
#include "sectStringArray.fdh"


/* decode the raw section data into the given StringList object */
bool SIFStringArraySect::Decode(const uint8_t *data, int datalen, StringList *out)
{
	int i;
	const uint8_t *data_end = data + (datalen - 1);
	int nstrings            = read_U16(&data, data_end);

	for(i = 0;i < nstrings; i++)
	{
		DString string;
		/* Section corrupt: overran end of data */
		if (data > data_end)
			return 1;

		ReadPascalString(&data, data_end, &string);

		out->AddString(string.String());
	}

	return 0;
}



uint8_t *SIFStringArraySect::Encode(StringList *strings, int *datalen_out)
{
	DBuffer buf;

	if (strings->CountItems() > 65535)
		return NULL;
	
	buf.Append16(strings->CountItems());
	for(int i=0;;i++)
	{
		const char *str = strings->StringAt(i);
		if (!str) break;
		
		WritePascalString(str, &buf);
	}
	
	if (datalen_out)
		*datalen_out = buf.Length();
	return buf.TakeData();
}

/*
void c------------------------------() {}
*/

void SIFStringArraySect::ReadPascalString(const uint8_t **data, const uint8_t *data_end, DString *out)
{
	int len = read_U8(data, data_end);
	if (len == 255)
		len = read_U16(data, data_end);
	
	out->Clear();
	for(int i=0;i<len;i++)
		out->AppendChar(read_U8(data, data_end));
}

void SIFStringArraySect::WritePascalString(const char *str, DBuffer *out)
{
	int len = strlen(str);
	if (len > 65535) len = 65535;
	
	if (len < 255)
		out->Append8(len);
	else
	{
		out->Append8(0xff);
		out->Append16(len);
	}
	
	for(int j=0;j<len;j++)
		out->Append8(str[j]);
}
