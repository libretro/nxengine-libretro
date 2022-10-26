
#include "../nx.h"
#include "CredReader.h"
#include "CredReader.fdh"
#include "libretro_shared.h"

/*
CREDITS FORMAT (credit.tsc)
==============
[T]X	Text T with casts.pbm image X [on left?]
+X		Shift credits X spaces towards the RIGHT
!X		Play music X
-X		Paragraph X lines
fX:Y	Jump to location Y if flag X is set
jX		Jump to location X
lX		Location X
~		Fade music to low volume
/		[end?]
<<<		[comment?]

*/

/*static */ int CredReader::ReadNumber()
{
	int num = atoi(&data[dataindex]);
	while(isdigit(get())) ;
	unget();
	return num;
}


bool CredReader::ReadCommand(CredCommand *cmd)
{
   int ch, i;

	memset(cmd, 0, sizeof(CredCommand));
	cmd->type = -1;
	
        /* ReadNextCommand called but file is not loaded? */
	if (!data)
		return 1;
	
	for(;;)
	{
		ch = get();
		if (ch == '\r' || ch == '\n') continue;
		else break;
	}
	
	cmd->type = ch;
	switch(ch)
	{
		case CC_TEXT:
		{
			for(i=0;i<sizeof(cmd->text)-1;i++)
			{
				cmd->text[i] = get();
				if (cmd->text[i] == ']' || !cmd->text[i]) break;
			}
			
			cmd->text[i] = 0;
		}
		break;
		
		case CC_SET_XOFF:
		case CC_BLANK_SPACE:
		case CC_JUMP:
		case CC_LABEL:
		case CC_MUSIC:
		case CC_FLAGJUMP:
		case CC_FADE_MUSIC:
		case CC_END:
		break;
		
		default:
			cmd->type = -1;
			return 1;
	}
	
	if (isdigit(peek()))
		cmd->parm = ReadNumber();
	
	if (get() == ':')
		cmd->parm2 = ReadNumber();
	else
		unget();
	
	return 0;
}


void CredReader::Rewind()
{
	dataindex = 0;
}

/*
void c------------------------------() {}
*/

CredReader::CredReader()
{
	data = NULL;
	dataindex = 0;
}

bool CredReader::OpenFile(void)
{
	char fname[MAXPATHLEN];
#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif

	if (data)
		CloseFile();
	snprintf(fname, sizeof(fname), "%s%cCredit.tsc", data_dir, slash);

	data = tsc_decrypt(fname, &datalen);
	if (!data)
		return 1;

	dataindex = 0;
	return 0;
}

void CredReader::CloseFile()
{
	if (data)
	{
		free(data);
		data = NULL;
		datalen = 0;
	}
}

char CredReader::get()
{
	if (dataindex >= datalen)
		return 0;
	
	return data[dataindex++];
}

void CredReader::unget()
{
	if (dataindex > 0)
		dataindex--;
}

char CredReader::peek()
{
	if (dataindex >= datalen)
		return 0;
	
	return data[dataindex];
}
