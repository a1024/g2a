//g2_file.h - Implementation of file operations for Grapher 2A.
//Copyright (C) 2020  Ayman Wagih Mohsen
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

//
// Created by MSI on 10/5/2020.
//

#include		<fstream>
#include		<sys/stat.h>
#include		<sys/types.h>
#include		<errno.h>
#include		<dirent.h>
#include		"g2_common.h"
#include		"g2_file.h"

const char		*statedir=nullptr,//full path to the folder containing the state
				*statefilepath=nullptr;//full path to file containing the state data
const char*		alloc_str(std::string const &str)
{
	unsigned size=str.size();
	char *ret=(char*)malloc(size+1);
	memcpy(ret, str.c_str(), size);
	ret[size]='\0';
	return ret;
}
#define 		ALLOC_STR(pointer, str)		if(!(pointer))pointer=alloc_str(str)
void			init_directories()
{
	ALLOC_STR(statedir, std::string(appdatapath)+'/'+statefoldername);
	ALLOC_STR(statefilepath, std::string(statedir)+'/'+statefilename);
}

void			sys_check(int line)
{
	int error=errno;
	if(error)
		LOGERROR_LINE(line, "G2_FILE: %d: %s", error, strerror(error));
}

int				saveFile(const char *addr, const char *data, size_t size, bool binary)
{
	const char mode[]={'w', char(binary*'b'), '\0'};
	FILE *file=fopen(addr, mode);	//SYS_CHECK();//2: No such file or directory
//	FILE *file=fopen(addr, binary?"wb":"w");	SYS_CHECK();
	if(!file)
		LOGERROR("Failed to save %s", addr);
	else
	{
		int byteswritten=fwrite(data, 1, size, file);	//SYS_CHECK();//2: No such file or directory
		if(byteswritten!=size)
			LOGERROR("saved %d/%d bytes of %s", byteswritten, size, addr);
		//	LOGERROR("Failed to save %s", addr);
		fclose(file);	//SYS_CHECK();//2: No such file or directory
		return 0;
	}
#if 0
	std::ofstream file(addr, std::ios::out|std::ios::binary);
	if(file.is_open())
	{
		file.write(data, size);
		file.close();
		return 0;
	}
#endif
	return 1;
}
long			getfilesize(FILE *file)
{//https://www.cplusplus.com/reference/cstdio/fread/
	fseek(file , 0 , SEEK_END);	SYS_CHECK();
	long size=ftell(file);
	rewind(file);
	return size;
}
int				loadFile(const char *addr, char *&data, size_t &size, bool binary)
{
#if 0
	FILE *file=fopen(addr, binary?"rb":"r");
	int error=errno;
	const char *msg=strerror(error);
	if(file)
	{
		size=getfilesize(file);
		data=(char*)malloc(size);
		size_t bytesread=fread(data, 1, size, file);	SYS_CHECK();
		if(bytesread!=size)
			LOGERROR("Error reading %s", addr);
		fclose(file);	SYS_CHECK();
		return 0;
	}
	size=0, data=nullptr;
	return 1;
#endif
#if 1
	struct stat info={};
	if(!stat(addr, &info))
	{
		SYS_CHECK();
		size=info.st_size;
		data=(char*)malloc(size*sizeof(unsigned char));
		const char mode[]={'r', char(binary*'b'), '\0'};
		FILE *file=fopen(addr, mode);	SYS_CHECK();
	//	FILE *file=fopen(addr, binary?"rb":"r");
		if(file)
		{
			size_t bytesread=fread(data, 1, size, file);	SYS_CHECK();
			if(bytesread!=size)
				LOGERROR("Read %d/%d from %s", (int)bytesread, (int)size, addr);
			fclose(file);	SYS_CHECK();
			return 0;
		}
		else
			free(data);
		//std::ifstream file(addr, std::ios::in|std::ios::binary);
		//if(file.is_open())
		//{
		//	file.read((char*)data, size);
		//	file.close();
		//	return 0;
		//}
	}
	size=0, data=nullptr;
	return 1;
#endif
}
void			directorycontents(const char *dirpath, std::vector<std::string> &ret)
{//works, dirpath doesn't have slash at the end
	DIR *d=opendir(dirpath);	SYS_CHECK();//https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program/17683417
	if(d)
	{
		struct dirent *dir;
		while((dir=readdir(d)))
		{
			SYS_CHECK();
			LOGI("G2_CL: %s", dir->d_name);
			ret.push_back(dir->d_name);
		}
		SYS_CHECK();
		closedir(d);	SYS_CHECK();
	}
}
void			mkdir_firsttime(const char *dirpath)
{
	struct stat s;
	int doesntexist=stat(dirpath, &s);
	if(doesntexist)
	{
		auto fail=mkdir(dirpath, 0777);
		SYS_CHECK();
	}
}
unsigned 		hexstr2uint(const char *text)
{
	unsigned number=0;
	if(text)
	{
		for(unsigned k=0;k<8;++k)
		{
			unsigned nibble=0;
			if(text[k]>='0'&&text[k]<='9')
				nibble=char(text[k]-'0');
			else if(text[k]>='A'&&text[k]<='F')
				nibble=char(text[k]-'A');
			else if(text[k]>='a'&&text[k]<='f')
				nibble=char(text[k]-'a');
			else
				break;
			number|=nibble<<((7-k)<<2u);
		}
	}
	return number;
}
int				statefolder_readstate(char *&text, size_t &length)
{
	init_directories();
	mkdir_firsttime(statedir);
	return loadFile(statefilepath, text, length, false);//zero if found (success)
}
void			statefolder_deletecontents()
{
	init_directories();
	std::vector<std::string> contents;
	directorycontents(statedir, contents);
	for(int k=0, nitems=contents.size();k<nitems;++k)
	{
		const char *name=contents[k].c_str();
		if(strcmp(name, ".")&&strcmp(name, ".."))
		{
			LOGI("Deleting %s", name);
			int fail=remove((statedir+('/'+contents[k])).c_str());
			SYS_CHECK();
		}
	}
}
const char*		version2str()
{
	static char vstr[9]={};
	for(unsigned k=0;k<8;++k)
	{
		char nibble=char(g2_version>>((7-k)<<2u)&15u);
		vstr[k]=char((nibble>=10?'A':'0')+nibble);
	}
	return vstr;
}
void 			statefolder_writestate(const char *usertext)
{
	std::string str=version2str();
	str+='\n';
	if(usertext)
		str+=usertext;

	init_directories();
	saveFile(statefilepath, str.c_str(), str.size(), false);
}